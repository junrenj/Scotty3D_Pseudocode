function rasterize_line(va, vb, emit_fragment) :
    // Ensure this function is only used in flat interpolation mode
    if (flags & PipelineMask_Interp) != Pipeline_Interp_Flat:
assert(false, "rasterize_line should only be invoked in flat interpolation mode.")

    // Extract screen space coordinates
    v0 = va.fb_position
    v1 = vb.fb_position
    x0, y0, z0 = int(v0.x), int(v0.y), v0.z
    x1, y1, z1 = int(v1.x), int(v1.y), v1.z

    dx = x1 - x0
    dy = y1 - y0
    dz = z1 - z0

    stepX, stepY = 1, 1

    // Ensure x0 <= x1 by swapping endpoints if necessary
    if x1 < x0:
    swap(v0, v1)
    swap(x0, x1)
    swap(y0, y1)
    swap(z0, z1)
    dx, dy, dz = x1 - x0, y1 - y0, z1 - z0

    // Adjust step direction for y
    if y1 < y0:
    stepY = -1
    dy = abs(dy)

    // Calculate line slope
    slope = dy / dx

    // Error terms for Bresenham’s line algorithm
    errX = 2 * dy - dx
    errY = 2 * dx - dy

    dz = dz / max(dx, dy)

    x, y = x0, y0


    centerX = x + 0.5
    centerY = y + 0.5

    frag = Fragment()
    frag.fb_position = Vec3(centerX, centerY, z0)
    frag.derivatives.fill(Vec2(0.0, 0.0))
    frag.attributes = va.attributes

    // Bresenham - style error checking for stepping
    if errX < 0:
    errX += 2 * dy
    else:
    errX += 2 * (dy - dx)
    y += stepY

    if errY < 0 :
        errY += 2 * dx
    else:
    errY += 2 * (dx - dy)
    x += stepX

    z0 += dz