// This is just the pseudocode for barycentric triangle rasterization
// Function to rasterize a triangle given three vertices and an emission function for fragments.
function rasterize_triangle(va, vb, vc, emit_fragment) {
    // Compute bounding box
    minX = floor(min(va.x, vb.x, vc.x)), maxX = ceil(max(va.x, vb.x, vc.x))
    minY = floor(min(va.y, vb.y, vc.y)), maxY = ceil(max(va.y, vb.y, vc.y))

    // Edge function (cross product)
    function edge(a, b, p) 
    {
        e = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x)
        return (e == 0 && isTopLeft(a, b)) ? -10000 : e
    }

    // Check if an edge is top-left
    function isTopLeft(a, b) 
    {
        return (a.y == b.y && a.x > b.x) || (a.y < b.y)
    }

    // Compute area
    area = edge(va, vb, vc)
    if (area == 0) return  // Degenerate triangle

    // Iterate through bounding box
    for y from minY to maxY
    {
        for x from minX to maxX 
        {
            target = (x + 0.5, y + 0.5)
            alpha = edge(vb, vc, target) / area
            beta = edge(vc, va, target) / area
            gamma = 1 - alpha - beta

            // Check inside triangle
            if (!((alpha >= 0 && beta >= 0 && gamma >= 0) || (alpha <= 0 && beta <= 0 && gamma <= 0))) continue
            if (alpha == -10000 || beta == -10000 || gamma == -10000) continue

            // Interpolate attributes
            frag.fb_position = (target.x, target.y, alpha * va.z + beta * vb.z + gamma * vc.z)
            if (mode == "Flat") frag.attributes = va.attributes
            else if (mode == "Smooth")
                for i in attributes.size() frag.attributes[i] = alpha * va[i] + beta * vb[i] + gamma * vc[i]
            else if (mode == "Correct") 
            {
                wpInv = alpha * va.inv_w + beta * vb.inv_w + gamma * vc.inv_w
                wp = 1 / wpInv
                for i in attributes.size() frag.attributes[i] = wp * (alpha * va[i] * va.inv_w + beta * vb[i] * vb.inv_w + gamma * vc[i] * vc.inv_w)
            }

            emit_fragment(frag)
        }
    }
}


function derivative
    for i in range(derivative_count) 
    {
        fragments[0].derivatives[i] = (fragments[1].attributes[i] - fragments[0].attributes[i], fragments[2].attributes[i] - fragments[0].attributes[i])
    }

