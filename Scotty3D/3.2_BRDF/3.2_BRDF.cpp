Scatter Lambertian::scatter(RNG& rng, Vec3 out, Vec2 uv) const 
{
	//A3T4: Materials - Lambertian BSDF scattering
	//Select a scattered light direction at random from the Lambertian BSDF

	[[maybe_unused]] Samplers::Hemisphere::Cosine sampler; //this will be useful

	Scatter ret;
	//TODO: sample the direction the light was scatter from from a cosine-weighted hemisphere distribution:
	ret.direction = sampler.sample(rng);

	//TODO: compute the attenuation of the light using Lambertian::evaluate():
	ret.attenuation = evaluate(out, ret.direction, uv);

	return ret;
}

Spectrum Lambertian::evaluate(Vec3 out, Vec3 in, Vec2 uv) const 
{
	//A3T4: Materials - Lambertian BSDF evaluation
	// Compute the ratio of outgoing/incoming radiance when light from in_dir
	// is reflected through out_dir: (albedo / PI_F) * cos(theta).
	// Note that for Scotty3D, y is the 'up' direction.
	Spectrum color = albedo.lock()->evaluate(uv);
	Vec3 N = Vec3(0, 1, 0);
	float cos_thetha = dot(in.unit(), N);

	if (cos_thetha <= 0)
		return Spectrum{};

	color = (color / PI_F) * cos_thetha;

	return color;
}

float Lambertian::pdf(Vec3 out, Vec3 in) const 
{
	//A3T4: Materials - Lambertian BSDF probability density function
	// Compute the PDF for sampling in_dir from the cosine-weighted hemisphere distribution.
	[[maybe_unused]] Samplers::Hemisphere::Cosine sampler; //this might be handy!
	float pdf = sampler.pdf(in);
	return pdf;
}

float Hemisphere::Cosine::pdf(Vec3 dir) const 
{
	if (dir.y < 0.0f) return 0.0f;
	return dir.y / PI_F;
}

Spectrum Pathtracer::sample_indirect_lighting(RNG& rng, const Shading_Info& hit) 
{
	//A3T4: path tracing - indirect lighting

	//Compute a single-sample Monte Carlo estimate of the indirect lighting contribution
	// at a given ray intersection point.

	//NOTE: this function and sample_direct_lighting_task4() perform very similar tasks.

	//TODO: ask hit.bsdf to sample an in direction that would scatter out along hit.out_dir
	Materials::Scatter scatter = hit.bsdf.scatter(rng, hit.out_dir, hit.uv);
	//TODO: rotate that direction into world coordinates
	Vec3 dir = hit.object_to_world.rotate(scatter.direction);
	//TODO: construct a ray travelling in that direction
	// NOTE: be sure to reduce the ray depth! otherwise infinite recursion is possible
	Ray ray = Ray(hit.pos, dir, Vec2(EPS_F, FLT_MAX), hit.depth - 1);
	//TODO: trace() the ray to get the reflected light (the second part of the return value)
	//TODO: weight properly depending on the probability of the sampled scattering direction and set radiance

	Spectrum radiance = trace(rng, ray).second;
	if (!hit.bsdf.is_specular())
	{
		radiance *= scatter.attenuation / hit.bsdf.pdf(hit.out_dir, scatter.direction);
	}
	return radiance;
}

Spectrum Pathtracer::sample_direct_lighting_task4(RNG& rng, const Shading_Info& hit) 
{
	//A3T4: Pathtracer - direct light sampling (basic sampling)

	// This function computes a single-sample Monte Carlo estimate of the _direct_ lighting
	// at our ray intersection point by sampling the BSDF.

	//NOTE: this function and sample_indirect_lighting() perform very similar tasks.

	// Compute exact amount of light coming from delta lights:
	//  (these don't need to be sampled)
	Spectrum radiance = sum_delta_lights(hit);

	//TODO: ask hit.bsdf to sample an in direction that would scatter out along hit.out_dir
	Materials::Scatter scatter = hit.bsdf.scatter(rng, hit.out_dir, hit.uv);
	//TODO: rotate that direction into world coordinates
	Vec3 dir = hit.object_to_world.rotate(scatter.direction);
	//TODO: construct a ray travelling in that direction
	// NOTE: because we want emitted light only, can use depth = 0 for the ray
	Ray ray = Ray(hit.pos, dir, Vec2(EPS_F, FLT_MAX), 0);
	//TODO: trace() the ray to get the emitted light (first part of the return value)
	Spectrum emittedLight = trace(rng, ray).first;
	//TODO: weight properly depending on the probability of the sampled scattering direction and add to radiance
	if (!hit.bsdf.is_specular())
	{
		emittedLight *= scatter.attenuation / hit.bsdf.pdf(hit.out_dir, scatter.direction);
	}
	return radiance + emittedLight;
}

Spectrum Pathtracer::sample_direct_lighting_task6(RNG& rng, const Shading_Info& hit)
{
	//A3T6: Pathtracer - direct light sampling (mixture sampling)
	// TODO (PathTracer): Task 6

	// For task 6, we want to upgrade our direct light sampling procedure to also
	// sample area lights using mixture sampling.
	// 0. return specular case
	if (hit.bsdf.is_specular())
	{
		return sample_direct_lighting_task4(rng, hit);
	}

	Spectrum radiance = sum_delta_lights(hit);

	Vec3 dir_WS;
	bool isUseLighting = rng.coin_flip(0.5f);
	if (isUseLighting)
	{
		// 1.Area Light
		dir_WS = sample_area_lights(rng, hit.pos);
	}
	else
	{
		// 2.BSDF
		Materials::Scatter scatter = hit.bsdf.scatter(rng, hit.out_dir, hit.uv);
		dir_WS = hit.object_to_world.rotate(scatter.direction);
	}

	Ray shadow_Ray = Ray(hit.pos, dir_WS, Vec2(EPS_F, FLT_MAX), 0);
	Spectrum incomingLight = trace(rng, shadow_Ray).first;
	Vec3 dir_OS = hit.world_to_object.rotate(dir_WS);
	Spectrum emittedLight = hit.bsdf.evaluate(hit.out_dir, dir_OS.unit(), hit.uv);

	float pdf_Light = area_lights_pdf(hit.pos, dir_WS);
	float pdf_Bsdf = hit.bsdf.pdf(hit.out_dir, dir_OS);

	float weight = (pdf_Light + pdf_Bsdf) * 0.5f;
	Spectrum mixLight = incomingLight * emittedLight / weight;

	// Example of using log_ray():
	if constexpr (LOG_AREA_LIGHT_RAYS) {
		if (log_rng.coin_flip(0.001f)) log_ray(Ray(), 100.0f);
	}

	return radiance + mixLight;
}