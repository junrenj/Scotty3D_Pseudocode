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



function CatmarkSubdivide() :
    // Create maps to store new positions for vertices, edges, and faces
    vertex_positions = EMPTY_MAP<Vertex, Vec3>
    edge_vertex_positions = EMPTY_MAP<Edge, Vec3>
    face_vertex_positions = EMPTY_MAP<Face, Vec3>

    // Step 1: Compute new positions for face vertices ===
    for EACH face f IN mesh.faces:
    if f is NOT a boundary face :
        face_vertex_positions[f] = f.center() // Compute face centroid

    // Step 2: Compute new positions for edge vertices ===
    for EACH edge e IN mesh.edges :
        edge_position = e.center() // Compute edge midpoint
        if e is NOT on the boundary :
            // Compute the weighted average of surrounding face centers
            edge_position = (2 * edge_position + e.halfedge.face.center() + e.halfedge.twin.face.center()) / 4
            edge_vertex_positions[e] = edge_position

    // Step 3: Compute new positions for original vertices ===
     for EACH vertex v IN mesh.vertices:
        S = v.position // Store original position
        new_vertex_position = Vec3(0, 0, 0)
        h = v.halfedge // Get associated half-edge
        n = v.degree() // Number of edges connected to vertex

        if v is on the boundary :
            // Find the two adjacent boundary vertices
            do:
                if h.face is a boundary :
                break
                else:
                h = h.twin.next
            while h != v.halfedge

            v_next = h.twin.vertex.position // Next boundary vertex
            toH = h
            do :
                toH = toH.next
            while toH.next != h
            v_pre = toH.vertex.position // Previous boundary vertex

            // Compute new boundary vertex position using weighted averaging
            new_vertex_position = 0.125 * v_pre + 0.125 * v_next + 0.75 * S
        else :
            // Compute Q (average of face centroids)
            // Compute R (average of edge midpoints)
            h_start = h
            Q = Vec3(0, 0, 0)
            R = Vec3(0, 0, 0)
            count = 0
            do:
                Q += h_start.face.center()
                R += h_start.edge.center()
                count += 1
                h_start = h_start.twin.next
            while h_start != h

            Q /= count
            R /= count

            // Compute new vertex position using the Catmull-Clark formula
            new_vertex_position = (Q + 2 * R + (n - 3) * S) / n

            vertex_positions[v] = new_vertex_position

    
    // Step 4: Apply computed positions to mesh ===
    call catmark_subdivide_helper(vertex_positions, edge_vertex_positions, face_vertex_positions)






function LoopSubdivide() :
    // Step 1: Ensure all non-boundary faces are triangular
    for EACH face f IN mesh.faces :
        if f is a boundary face :
            continue
        if f has more than 3 edges :
            return false // The mesh contains non-triangular faces

        // If execution reaches this point, all non-boundary faces are triangular

    // Step 2: Compute new vertex positions using Loop subdivision rules
    vertex_new_pos = EMPTY_MAP<Vertex, Vec3>

    for EACH vertex v IN mesh.vertices:
        h = v.halfedge
        num = 0
        v_old = v.position
        v_new = Vec3(0, 0, 0)
        neighbor_sum = Vec3(0, 0, 0)
        is_boundary_vertex = false

        // Traverse the one-ring neighborhood of v
        do:
            if h.edge is on the boundary :
                is_boundary_vertex = true

            neighbor_sum += h.twin.vertex.position
            h = h.twin.next
            num++
        while h != v.halfedge

        if is_boundary_vertex :
            neighbor_sum = Vec3(0, 0, 0)
            h1 = h
                do :
                    if h1.edge is on the boundary :
                        neighbor_sum += h1.twin.vertex.position
                    h1 = h1.twin.next
                while h1 != h

                v_new = 0.125 * neighbor_sum + 0.75 * v_old

        else :
            if num == 3 :
                v_new = (1 - 3 * num / 16) * v_old + (3 / 16) * neighbor_sum
            else :
                v_new = (5 / 8) * v_old + (3 / (8 * num)) * neighbor_sum

         vertex_new_pos[v] = v_new

    // Step 3: Compute new positions for edge split vertices
    edge_new_pos = EMPTY_MAP<Edge, Vec3>

    for EACH edge e IN mesh.edges:
        A = e.halfedge.vertex.position
        B = e.halfedge.twin.vertex.position

        if e is on the boundary :
            newV = (A + B) / 2
        else :
            C = e.halfedge.next.next.vertex.position
            D = e.halfedge.twin.next.next.vertex.position
            newV = 0.375 * (A + B) + 0.125 * (C + D)

        edge_new_pos[e] = newV

    // Step 4: Split every edge in the original mesh
    new_edges = EMPTY_LIST
    original_edges = LIST_OF_ALL_EDGES()

    for EACH edge e IN original_edges :
        new_vertex = SPLIT_EDGE(e)
        if e IN edge_new_pos :
            new_vertex.position = edge_new_pos[e]

        if new_vertex is NOT on the boundary :
            new_edges.APPEND(new_vertex.halfedge.twin.next.edge)

        new_edges.APPEND(new_vertex.halfedge.next.next.edge)

    // Step 5: Flip new edges that connect a new and old vertex
    function is_new(vertex v) :
        return v is NOT in vertex_new_pos

    for EACH edge e IN new_edges :
        v0 = e.halfedge.vertex
        v1 = e.halfedge.twin.vertex

        if(NOT is_new(v0) AND is_new(v1)) OR(is_new(v0) AND NOT is_new(v1)) :
            FLIP_EDGE(e)

    // Step 6: Copy new vertex positions into the mesh
    for EACH vertex v IN mesh.vertices :
        if v IN vertex_new_pos :
            v.position = vertex_new_pos[v]

    return true
