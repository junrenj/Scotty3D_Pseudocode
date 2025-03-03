// This is just the Pseudocode of logic, not the real code

FUNCTION rasterize_line(va, vb, emit_fragment) :
    IF(flags& PipelineMask_Interp) ≠ Pipeline_Interp_Flat THEN
    ASSERT ERROR "rasterize_line should only be invoked in flat interpolation mode."

    // 获取屏幕空间坐标
    v0 = va.fb_position
    v1 = vb.fb_position
    x0, y0, z0 = v0.x, v0.y, v0.z
    x1, y1, z1 = v1.x, v1.y, v1.z

    dx = x1 - x0
    dy = y1 - y0
    dz = z1 - z0

    stepX = 1
    stepY = 1
    isSwap = FALSE

    // 确保 x1 > x0
    IF x1 < x0 THEN
    SWAP(v0, v1)
    SWAP(x0, x1)
    SWAP(y0, y1)
    SWAP(z0, z1)
    dx = x1 - x0
    dy = y1 - y0
    dz = z1 - z0
    isSwap = TRUE

    // 确保 y 方向步进
    IF y1 < y0 THEN
    stepY = -1
    dy = ABS(dy)

    slope = dy / dx

    errX = 2 * dy - dx
    errY = 2 * dx - dy

    dz = dz / MAX(dx, dy)

    x = x0
    y = y0

    FUNCTION FloatEqual(a, target) :
    RETURN ABS(a - target) ≤ 1e-9

    WHILE TRUE :
centerX = x + 0.5
centerY = y + 0.5

frag.fb_position = (centerX, centerY, z0)
frag.derivatives = (0, 0)
frag.attributes = va.attributes

IF x = x0 AND y = y0 THEN
IF isSwap AND DiamondExitEnd(x, y, v0.x, v0.y, (v1.x - v0.x) / (v1.y - v0.y)) THEN
CALL emit_fragment(frag)

IF x = x1 AND y = y1 THEN
IF NOT isSwap AND DiamondExitEnd(x, y, v1.x, v1.y, slope) THEN
CALL emit_fragment(frag)
ELSE
CALL emit_fragment(frag)
BREAK
ELSE
CALL emit_fragment(frag)

// X 方向主导
IF errX < 0 THEN
    errX += 2 * dy
    ELSE
    errX += 2 * (dy - dx)
    y += stepY

    // Y 方向主导
    IF errY < 0 THEN
    errY += 2 * dx
    ELSE
    errY += 2 * (dx - dy)
    x += stepX

    z0 += dz
