// This is just the pseudocode for barycentric triangle rasterization
// Function to rasterize a triangle given three vertices and an emission function for fragments.
function rasterize_triangle(va, vb, vc, emit_fragment) {
    // Compute bounding box for the triangle
    minX = floor(min(va.fb_position.x, vb.fb_position.x, vc.fb_position.x))
        minY = floor(min(va.fb_position.y, vb.fb_position.y, vc.fb_position.y))
        maxX = ceil(max(va.fb_position.x, vb.fb_position.x, vc.fb_position.x))
        maxY = ceil(max(va.fb_position.y, vb.fb_position.y, vc.fb_position.y))

        // Helper function to determine if an edge is a top-left edge
        function isTopLeftEdge(a, b) {
        return (a.y == b.y && a.x > b.x) || (a.y < b.y)
    }

    // Function to compute edge function (cross product)
    function edgeFunction(a, b, point) {
        edgeValue = (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x)

            // If the point lies exactly on the edge, apply the top-left rule
            if (edgeValue == 0 && isTopLeftEdge(a, b)) {
                edgeValue = -10000  // Special marker to discard pixel
            }
        return edgeValue
    }

    // Define the triangle vertices as 2D points
    a = (va.fb_position.x, va.fb_position.y)
        b = (vb.fb_position.x, vb.fb_position.y)
        c = (vc.fb_position.x, vc.fb_position.y)

        // Compute triangle area using edge function
        area = edgeFunction(a, b, c)

        // If area is zero, the vertices form a degenerate triangle (a line or a point), so exit
        if (area == 0) return

            // Loop through every pixel in the bounding box
            for y from minY to maxY{
                for x from minX to maxX {
                    fragments = array of 3 fragments
                    isValidPixel = false

                    // Compute for 2x2 block derivatives
                    for j from 0 to 2 {
                        target = (x + 0.5, y + 0.5)
                        if (j == 1) target = target + (1.0, 0.0)  // Right pixel
                        if (j == 2) target = target + (0.0, 1.0)  // Bottom pixel

                            // Compute barycentric coordinates
                            alpha = edgeFunction(b, c, target)
                            beta = edgeFunction(c, a, target)
                            gamma = edgeFunction(a, b, target)

                            alpha /= area
                            beta /= area
                            gamma = 1 - alpha - beta

                            // Interpolate depth value
                            fragments[j].fb_position = (target.x, target.y, alpha * va.fb_position.z + beta * vb.fb_position.z + gamma * vc.fb_position.z)

                            // Attribute interpolation based on the pipeline mode
                            if (pipeline_mode == "Flat") {
                                fragments[j].attributes = va.attributes
                            }
                            else if (pipeline_mode == "Smooth") {
                                for i in range(attributes_count) {
                                    fragments[j].attributes[i] = alpha * va.attributes[i] + beta * vb.attributes[i] + gamma * vc.attributes[i]
                                }
                            }
                            else if (pipeline_mode == "Correct") {
                                // Perspective-correct interpolation
                                wpInverse = alpha * va.inv_w + beta * vb.inv_w + gamma * vc.inv_w
                                wp = 1 / wpInverse
                                for i in range(attributes_count) {
                                    attribute = alpha * va.attributes[i] * va.inv_w + beta * vb.attributes[i] * vb.inv_w + gamma * vc.attributes[i] * vc.inv_w
                                    fragments[j].attributes[i] = wp * attribute
                                }
                            }

                        // Check if pixel is inside the triangle
                        if (j == 0) {
                            if ((alpha >= 0 && beta >= 0 && gamma >= 0) || (alpha <= 0 && beta <= 0 && gamma <= 0)) {
                                isValidPixel = true
                            }
                            if (alpha == -10000 || beta == -10000 || gamma == -10000) {
                                isValidPixel = false
                            }
                        }
                    }

    // Compute derivatives using neighboring pixels
    for i in range(derivative_count) {
        fragments[0].derivatives[i] = (fragments[1].attributes[i] - fragments[0].attributes[i], fragments[2].attributes[i] - fragments[0].attributes[i])
    }

    // Emit the fragment if it passes validation
    if (isValidPixel) {
        emit_fragment(fragments[0])
    }
}
            }
}
