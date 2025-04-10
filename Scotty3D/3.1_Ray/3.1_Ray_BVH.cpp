std::pair<Ray, float> Camera::sample_ray(RNG& rng, uint32_t px, uint32_t py) 
{
	Samplers::Rect s;
	Vec2 offset = s.sample(rng);
	float offset_pdf = s.pdf(offset);
	Vec2 sensor_pixel = Vec2(float(px), float(py)) + offset;

	// 将传感器像素转换为传感器平面上的世界坐标位置
	float h = 2.0f * std::tan(Radians(vertical_fov) * 0.5f);
	float w = h * aspect_ratio;
	float posX = -w * 0.5f + sensor_pixel.x / (float)film.width * w;
	float posY = -h * 0.5f + sensor_pixel.y / (float)film.height * h;

	// 构建射线
	Ray ray;
	ray.point = Vec3(); // 射线从原点出发
	ray.dir = Vec3(posX, posY, -1.0f); // 赋予方向值
	ray.depth = film.max_ray_depth; // 射线的最大深度

	return { ray, offset_pdf };
}


Trace Triangle::hit(const Ray& ray) const 
{

	auto CheckIsInRange = [](float p, float min, float max)->bool
		{
			return p >= min && p <= max;
		};

	// 每一个顶点都有自己的位置信息和法线信息
	Tri_Mesh_Vert v_0 = vertex_list[v0];
	Tri_Mesh_Vert v_1 = vertex_list[v1];
	Tri_Mesh_Vert v_2 = vertex_list[v2];

	// 与三个顶点构成的表面进行射线求交
	Vec3 O = ray.point;
	Vec3 D = ray.dir;
	Vec3 E1 = v_1.position - v_0.position;
	Vec3 E2 = v_2.position - v_0.position;
	Vec3 N = cross(E2, E1);
	Vec3 S = O - v_0.position;
	Vec3 S1 = cross(D, E2);
	Vec3 S2 = cross(S, E1);

	// Moller-Trumbore 算法
	float S1_E1 = dot(S1, E1);

	Trace ret;
	if (fabs(S1_E1) < 1e-6f)
		return ret;

	float inv_S1_E1 = 1.0f / S1_E1;
	float u = dot(S1, S) * inv_S1_E1;
	if (!CheckIsInRange(u, 0.0f, 1.0f))
		return ret;

	float v = dot(S2, D) * inv_S1_E1;
	if (!CheckIsInRange(v, 0.0f, 1.0f))
		return ret;

	float t = dot(S2, E2) * inv_S1_E1;
	if (!CheckIsInRange(t, ray.dist_bounds.x, ray.dist_bounds.y))
		return ret;

	float w = 1.0f - u - v;
	if (!CheckIsInRange(w, 0.0f, 1.0f))
		return ret;

	// 赋予值
	ret.origin = O;
	ret.hit = true;
	ret.distance = t;
	ret.position = O + t * D;
	ret.normal = (v_0.normal * w + v_1.normal * u + v_2.normal * v).unit();
	ret.uv = v_0.uv * w + v_1.uv * u + v_2.uv * v;

	return ret;
}

PT::Trace Sphere::hit(Ray ray) const 
{
	// 检查是否在范围内
	auto CheckInDis_Bound = [](float t, Vec2 dis_bound)->bool
	{
		return t >= dis_bound.x && t <= dis_bound.y;
	};

	// 球体求交
	Vec3 O = ray.point;
	Vec3 D = ray.dir;
	float a = D.norm() * D.norm();
	float b = 2 * dot(O, D);
	float c = O.norm() * O.norm() - radius * radius;
	float delta = b * b - 4.0f * a * c;
	float t1 = (-b - sqrt(delta)) / (2.0f * a);
	float t2 = (-b + sqrt(delta)) / (2.0f * a);

	// 检查两个交点
	float t = 0.0f;
	bool hasIntersectPoint = false;
	if (CheckInDis_Bound(t1, ray.dist_bounds))
	{
		t = t1;
		hasIntersectPoint = true;
	}
	else if (CheckInDis_Bound(t2, ray.dist_bounds))
	{
		t = t2;
		hasIntersectPoint = true;
	}

	// 赋予值
	PT::Trace ret;
	ret.origin = ray.point;
	ret.hit = (delta >= 0.0f) && hasIntersectPoint;
	ret.distance = t;
	ret.position = O + t * D;
	ret.normal = ret.position.unit();
	ret.uv = uv(ret.position);
	return ret;
}

bool hit(const Ray& ray, Vec2& times) const {
	// 获取基础数据
	float px = ray.point.x;	float py = ray.point.y;	float pz = ray.point.z;
	float inv_Dx = 1.0f / ray.dir.x;	float inv_Dy = 1.0f / ray.dir.y;	float inv_Dz = 1.0f / ray.dir.z;
	float min_x = min.x;		float max_x = max.x;
	float min_y = min.y;		float max_y = max.y;
	float min_z = min.z;		float max_z = max.z;

	// 获取x
	float tmin = (min_x - px) * inv_Dx;
	float tmax = (max_x - px) * inv_Dx;
	if (inv_Dx < 0)
		std::swap(tmin, tmax);

	float tymin = (min_y - py) * inv_Dy;
	float tymax = (max_y - py) * inv_Dy;

	if (inv_Dy < 0)
		std::swap(tymin, tymax);

	// 与y相比
	if ((tmin > tymax) || (tymin > tmax))
		return false;

	tmin = tmin < tymin ? tymin : tmin;
	tmax = tmax > tymax ? tymax : tmax;

	float tzmin = (min_z - pz) * inv_Dz;
	float tzmax = (max_z - pz) * inv_Dz;
	if (inv_Dz < 0)
		std::swap(tzmin, tzmax);

	// 与z相比
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	tmin = tmin < tzmin ? tzmin : tmin;
	tmax = tmax > tzmax ? tzmax : tmax;

	if ((tmax < times.x) || (tmin > times.y))
		return false;

	times.x = std::max(times.x, tmin);
	times.y = std::min(times.y, tmax);

	return true;
}

template<typename Primitive>
void BVH<Primitive>::build(std::vector<Primitive>&& prims, size_t max_leaf_size) {
	// 清空数据 写入片元
	nodes.clear();
	primitives = std::move(prims);

	if (primitives.empty())
		return;

	// 构建BVH主函数
	std::function<void(BVHBuildData, size_t)> build_repeated;
	build_repeated = [&](BVHBuildData data, size_t max_leaf_size)
		{
			if (data.range <= max_leaf_size)
				return;

			size_t index = data.node;

			// 第一步：创建一个包围盒
			BBox bb = nodes[index].bbox;

			size_t leftSize_best = 0;
			size_t rightSize_best = 0;
			BBox left_bb_best;
			BBox right_bb_best;
			int axis_best = 0;
			float partition_best = 0.0f;
			float cost_lowest = FLT_MAX;
			size_t bucket_num = 16;

			// 第二步 找出主轴
			for (int axis = 0; axis < 3; axis++)
			{
				float bucket_w = (bb.max - bb.min)[axis] / float(bucket_num);
				std::vector<SAHBucketData> buckets(bucket_num);
				if (bucket_w < 1e-6f)
					bucket_w = 1e-6f;
				// 第三步 将片元放到不同的桶里面
				for (size_t p_idx = data.start; p_idx < data.start + data.range; p_idx++)
				{
					float center = primitives[p_idx].bbox().center()[axis];
					size_t bucket_idx = static_cast<size_t>((center - bb.min[axis]) / bucket_w);

					if (bucket_idx >= bucket_num)
						bucket_idx = bucket_num - 1;

					buckets[bucket_idx].bb.enclose(primitives[p_idx].bbox());
					buckets[bucket_idx].num_prims++;
				}

				// 第四步 找到最好的桶分割选项
				for (size_t bucketSplit_idx = 1; bucketSplit_idx < bucket_num; bucketSplit_idx++)
				{
					BBox left_bb;
					BBox right_bb;
					size_t N_left = 0;
					size_t N_right = 0;

					// 计算左边包围盒
					for (size_t l = 0; l < bucketSplit_idx; l++)
					{
						left_bb.enclose(buckets[l].bb);
						N_left += buckets[l].num_prims;
					}
					// 计算右边包围盒
					for (size_t r = bucketSplit_idx; r < bucket_num; r++)
					{
						right_bb.enclose(buckets[r].bb);
						N_right += buckets[r].num_prims;
					}
					float cost = left_bb.surface_area() * (float)N_left + right_bb.surface_area() * (float)N_right;

					if (cost < cost_lowest)
					{
						cost_lowest = cost;
						leftSize_best = N_left;
						rightSize_best = N_right;
						left_bb_best = left_bb;
						right_bb_best = right_bb;
						partition_best = bb.min[axis] + bucket_w * bucketSplit_idx;
						axis_best = axis;
					}
				}
			}

			// 第五步 重新排序
			std::partition(primitives.begin() + data.start, primitives.begin() + data.start + data.range,
				[axis_best, partition_best](Primitive& p)
				{
					return p.bbox().center()[axis_best] <= partition_best;
				});

			// 第六步 创建新的子节点
			nodes[index].l = new_node(left_bb_best, data.start, leftSize_best, 0, 0);
			nodes[index].r = new_node(right_bb_best, data.start + leftSize_best, rightSize_best, 0, 0);

			// 第七步 重复递归
			BVHBuildData left_data = BVHBuildData(data.start, leftSize_best, nodes[index].l);
			build_repeated(left_data, max_leaf_size);
			BVHBuildData right_data = BVHBuildData(data.start + leftSize_best, rightSize_best, nodes[index].r);
			build_repeated(right_data, max_leaf_size);
		};

	// 主函数
	BBox bb_root;
	for (size_t i = 0; i < primitives.size(); i++)
	{
		bb_root.enclose(primitives[i].bbox());
	}

	new_node(bb_root, 0, primitives.size(), 0, 0);
	BVHBuildData newData = BVHBuildData(0, primitives.size(), 0);
	build_repeated(newData, max_leaf_size);

}


template<typename Primitive> Trace BVH<Primitive>::hit(const Ray& ray) const {
	// 遍历BVH
	Trace t;
	Vec2 times = ray.dist_bounds;
	if (nodes.empty())
		return t;

	// 找到最近的相交位置
	auto FindClosestHit = [&](auto& self, const Node& node, const Ray& ray, Vec2 newTime)-> Trace
		{
			Trace ret;
			ret.distance = FLT_MAX;
			if (node.bbox.hit(ray, newTime))
			{
				if (node.is_leaf())
				{
					for (size_t i = node.start; i < node.start + node.size; i++)
					{
						Trace newRet = primitives[i].hit(ray);
						if (newRet.hit && newRet.distance < ret.distance)
							ret = newRet;
					}
					return ret;
				}
				else
				{
					size_t first = node.l;
					size_t second = node.r;
					bool leftHit;
					bool rightHit;
					Vec2 localTimes_L = newTime;
					Vec2 localTimes_R = newTime;
					leftHit = nodes[first].bbox.hit(ray, localTimes_L);
					rightHit = nodes[second].bbox.hit(ray, localTimes_R);

					// 情况1：两个包围盒都有相交
					if (leftHit && rightHit)
					{
						// 先检测最近的包围盒
						if (localTimes_L.x > localTimes_R.x)
						{
							std::swap(first, second);
							std::swap(localTimes_L, localTimes_R);
						}

						Trace leftTrace = self(self, nodes[first], ray, localTimes_L);
						if (leftTrace.hit)
							ret = leftTrace;

						Trace rightTrace = self(self, nodes[second], ray, localTimes_R);
						if (rightTrace.hit && rightTrace.distance <= ret.distance)
							ret = rightTrace;
					}
					else if (leftHit)	// 只有左边相交了
						ret = self(self, nodes[first], ray, localTimes_L);
					else if (rightHit)	// 只有右边相交了
						ret = self(self, nodes[second], ray, localTimes_R);
					return ret;
				}
			}
			else
				return ret;
		};

	// 主逻辑
	t = FindClosestHit(FindClosestHit, nodes[0], ray, times);
	return t;
}