// This is just the pseudocode for Lod Caculation

function CaculateLod
{
	// Compute texture coordinate derivatives
	dudx = fdx_texcoord.x * wh.x
	dvdx = fdx_texcoord.y * wh.y
	dudy = fdy_texcoord.x * wh.x
	dvdy = fdy_texcoord.y * wh.y

	// Compute the maximum rate of change (L)
	L = sqrt(max(dudx * dudx + dvdx * dvdx, dudy * dudy + dvdy * dvdy))

	// Compute level of detail (LOD) using log2
	lod = log2(L)
}





// This is just the pseudocode for Mipmap Generation
function DownsampleMipMap
{
    dstW = dst.w
    dstH = dst.h
    array dstPixels[dstW * dstH]

    for y from 0 to dstH - 1:
        for x from 0 to dstW - 1 :
            x0 = 2 * x
            y0 = 2 * y
            x1 = min(x0 + 1, src.w - 1)
            y1 = min(y0 + 1, src.h - 1)

            index_1 = y0 * src.w + x0
            index_2 = y0 * src.w + x1
            index_3 = y1 * src.w + x0
            index_4 = y1 * src.w + x1
            index_dst = y * dstW + x

            dstPixels[index_dst] = (src.data[index_1] + src.data[index_2] +
                                    src.data[index_3] + src.data[index_4]) / 4

    dst = HDR_Image(dstW, dstH, dstPixels)
}




// This is just the pseudocode for Sample_nearest
function Sample_nearest
{
    // Clamp UV coordinates to [0,1] and convert to pixel space
    x = image.w * clamp(uv.x, 0.0, 1.0)
    y = image.h * clamp(uv.y, 0.0, 1.0)

    // Find the pixel whose center is closest to (x, y)
    ix = floor(x)
    iy = floor(y)

    // Ensure indices are within valid bounds [0, w-1] and [0, h-1]
    ix = min(ix, image.w - 1)
    iy = min(iy, image.h - 1)

    // Return the sampled pixel value
    return image.at(ix, iy)

}

// This is just the pseudocode for Sample_bilinear
function Sample_bilinear
{
    // Convert UV coordinates to pixel space and adjust to align with pixel centers
    x = image.w * clamp(uv.x, 0.0, 1.0) - 0.5
    y = image.h * clamp(uv.y, 0.0, 1.0) - 0.5

    // Find the lower (x0, y0) and upper (x1, y1) bounding pixels
    x0 = max(0, floor(x))
    y0 = max(0, floor(y))

    // Ensure indices stay within valid image bounds
    x0 = min(x0, image.w - 1)
    y0 = min(y0, image.h - 1)
    x1 = min(x0 + 1, image.w - 1)
    y1 = min(y0 + 1, image.h - 1)

    // Compute interpolation weights
    tx = x - x0
    ty = y - y0

    // Perform bilinear interpolation:
    // First, interpolate between left and right pixels in the x-direction
    col0 = image.at(x0, y0) * (1 - tx) + image.at(x1, y0) * tx
    col1 = image.at(x0, y1) * (1 - tx) + image.at(x1, y1) * tx

    // Then, interpolate between the two results in the y-direction
    finalCol = col0 * (1 - ty) + col1 * ty

    // Return the final interpolated color
    return finalCol
}








// This is just the pseudocode for Sample_trilinear
function Sample_trilinear
{
    // Clamp the LOD (level of detail) value within the valid mip-map range
    goodLod = clamp(lod, 0.0, size(levels))

    // If LOD is at the base level, use bilinear sampling directly
    if goodLod == 0.0:
        return sample_bilinear(base, uv)

    // Determine the two mip levels to interpolate between
    low = floor(goodLod)
    high = min(low + 1, size(levels))

    // Compute interpolation factor
    t = goodLod - low

    // Sample from the two mip levels
    s1 = if goodLod < 1.0 then sample_bilinear(base, uv) else sample_bilinear(levels[low - 1], uv)
    s2 = sample_bilinear(levels[high - 1], uv)

    // Linearly interpolate between the two mip levels
    return s1 * (1 - t) + s2 * t

}

