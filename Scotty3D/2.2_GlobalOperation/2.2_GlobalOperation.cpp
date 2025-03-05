function triangulate() :

    // Helper function to project 3D coordinates to 2D
    function Vec3ToVec2(position, coordinatePlane) :
        if coordinatePlane is XY :
            return(position.x, position.y)
        else if coordinatePlane is YZ :
            return(position.y, position.z)
        else if coordinatePlane is ZX :
            return(position.z, position.x)
        else :
            return(position.x, position.y)

    // Helper function to check if a triangle is convex
    function isConvex(previous, next, current, isCounterclockwise) :
        crossProduct = (current.x - previous.x) * (next.y - current.y)
                        - (next.x - current.x) * (current.y - previous.y)
        if isCounterclockwise
            return (crossProduct > 0)
        else
            return (crossProduct < 0)

    // Helper function to check if a point lies inside a triangle using barycentric coordinates
    function isInTriangle(point, a, b, c) :
        alpha = (c.x - b.x) * (point.y - b.y) - (point.x - b.x) * (c.y - b.y)
        beta = (a.x - c.x) * (point.y - c.y) - (point.x - c.x) * (a.y - c.y)
        gamma = (b.x - a.x) * (point.y - a.y) - (point.x - a.x) * (b.y - a.y)
        return(all positive) or (all negative)

    // Function to remove an ear from the polygon
    function deleteEar(face, halfedgeEar, halfedgePrev, halfedgePrevPrev) :
        newFace = create new face
        newHalfedge1, newHalfedge2 = create two new halfedges
        newEdge = create new edge connecting newHalfedge1 and newHalfedge2

        Assign newEdge to newHalfedges
        Assign newHalfedges to correct face
        Assign newFace to halfedges in the new region
        Update halfedge connectivity in the loop

    // Function to check if a given vertex is an ear
    function isEar(iterator, coordinatePlane, halfedgeLoop, isCounterclockwise) :
        previous = previous halfedge in loop
        next = next halfedge in loop
        a, b, c = project previous, current, next vertices to 2D

        if not isConvex(a, b, c, isCounterclockwise) :
        return false

        // Check if any other vertex is inside the triangle
        for each halfedge in halfedgeLoop :
        if vertex(excluding a, b, c) is inside triangle :
            return false

        return true

    // Ear Clipping Algorithm
    function earClipping(face) :
        halfedgesLoop = list of halfedges forming the face loop

        if face has only 3 vertices :
        Return

        // Determine the best 2D plane for projection
        normal = compute normal of face
        if abs(normal.x) is largest :
            coordinatePlane = YZ
            isCounterclockwise = normal.x > 0
        else if abs(normal.y) is largest :
        coordinatePlane = ZX
            isCounterclockwise = normal.y > 0
        else:
            coordinatePlane = XY
        isCounterclockwise = normal.z > 0

        // Find initial set of ears
        earList = []
        for each halfedge in halfedgesLoop :
            if isEar(halfedge, coordinatePlane, halfedgesLoop, isCounterclockwise) :
                Add to earList

            // Remove ears iteratively until only one triangle remains
            While halfedgesLoop has more than 3 elements :
            Select the first ear from earList
            Remove it by calling deleteEar()
            Remove the halfedge from the loop
            Recalculate ear candidates to update earList

    // Iterate over all faces and apply triangulation
    for each face in mesh :
        If face is a boundary or has only 3 vertices :
            continue

        Call earClipping(face)






function linear_subdivide() :
    // Maps to store new vertex positions
    vertex_positions = empty map(Vertex.Position)
    edge_vertex_positions = empty map(Edge.Position)
    face_vertex_positions = empty map(Face.Position)

    // Step 1: Store original vertex positions
    for each vertex v in mesh :
        vertex_positions[v] = v.position

    // Step 2: Compute midpoint for each edge
    for each edge e in mesh :
        edge_vertex_positions[e] = midpoint of e adjacent vertices

    // Step 3: Compute centroid for each non-boundary face
    for each face f in mesh :
        if f is not a boundary :
            face_vertex_positions[f] = centroid of face f


    // Step 4: Perform subdivision using computed positions
    Call catmark_subdivide_helper(vertex_positions, edge_vertex_positions, face_vertex_positions)
