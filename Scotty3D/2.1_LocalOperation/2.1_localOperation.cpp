function flip_edge(edge e) :

    // Special Case 1: If the edge is on the boundary, return null
    if e.is_boundary() :
        return null

    // Step 1: Collect necessary data
    h, t = e.halfedge, e.halfedge.twin
    v1, v2 = h.vertex, t.vertex
    v3, v4 = h.next.next.vertex, t.next.next.vertex
    f1, f2 = h.face, t.face

    // Special Case 2: If v3 and v4 are directly connected, return null
    if v3.is_connected_to(v4) :
        return null

    // Step 2: Find previous half-edges before h and t
    toH = find_previous_halfedge(h)
    toT = find_previous_halfedge(t)

    // Special Case 3: If v1 or v2 has only two connected edges, return null
    if v1.degree() <= 2 or v2.degree() <= 2:
return null

// Step 3: Rewire half-edges to perform the edge flip
toH.next = t.next  // Replace h with t.next
toT.next = h.next  // Replace t with h.next
t_next, h_next = t.next, h.next

// Step 4: Update vertex connections
v1.halfedge = t.next
v2.halfedge = h.next

// Step 5: Swap edge vertices
t.vertex = v3
h.vertex = v4
h.next = h.next.next
t.next = t.next.next
h_next.next = t
t_next.next = h

// Step 6: Update face ownership
assign_face(h, f1)
assign_face(t, f2)

return e // Return flipped edge reference









function split_edge(edge e) :

    // Step 1: Collect necessary data
    h = e.halfedge
    t = h.twin
    v1, v2 = h.next.vertex, t.next.vertex
    v3, v4 = h.next.next.vertex, t.next.next.vertex
    f1, f2 = h.face, t.face

    // Find the previous half - edges before h and t
    toH_Old = find_previous_halfedge(h)
    toT_Old = find_previous_halfedge(t)

    // Step 2: Create a new vertex at the midpoint
    newV = create_vertex()
    newV.position = (v1.position + v2.position) / 2
    interpolate_data({ v1, v2 }, newV)

    // Step 3: Create new half - edges and an edge to split h & t
    newEdge = create_edge()
    h_half, t_half = create_halfedge_pair()

    // Link new half - edges
    link_twins(h_half, t_half)
    link_edge(newEdge, h_half, t_half)

    // Update vertex references
    h.vertex = newV
    h_half.vertex = v2
    t_half.vertex = newV
    v2.halfedge = h_half

    // Update half - edge connections
    h_half.next = h
    t_half.next = t.next
    t.next = t_half
    toH_Old.next = h_half
    newV.halfedge = h
    t_half.face = t.face

    // Step 4: Connect new vertex to v3 and v4 if faces exist
    if f1 is not boundary :
        connect_vertex(newV, v3, h, h_half, f1)

    if f2 is not boundary :
        connect_vertex(newV, v4, t_half, t, f2)

        return newV


function collapse_edge(edge e) :

    // Step 1: Collect data
    h, t = e.halfedge, e.halfedge.twin
    v0, v1 = h.vertex, t.vertex
    f1, f2 = h.face, t.face
    toH, toT = find_previous_halfedge(h), find_previous_halfedge(t)

    // Special Case 1: If the edge forms a single triangle, return null
    if h.next.twin == toT and t.next.twin == toH:
        return null

    // Special Case 2: If the edge is non-manifold, return null
    if not e.is_boundary() and v0.is_boundary() and v1.is_boundary() :
        return null

    // Step 2: Move v0 to the midpoint of v0 and v1
    v0.position = (v0.position + v1.position) / 2.0
    interpolate_data([v0, v1], v0)  // Merge UVs, normals, etc.

    // Step 3: Reconnect half-edges
    toT.next = t.next
    toH.next = h.next
    h.next.vertex = v0
    h.next.twin.next.vertex = v0
    toT.twin.vertex = v0

    // Update face references
    f1.halfedge = h.next
    f2.halfedge = t.next
    v0.halfedge = h.next

    // Step 4: Handle degenerate faces
    function check_erase_face(face fDelete, vertex vDelete) :
    targetH = fDelete.halfedge
    if fDelete.degree() < 3 :  // Face collapses into a single edge
        targetTwin, targetNextTwin = targetH.twin, targetH.next.twin
        v3 = targetH.next.vertex
        eDelete = targetH.edge

    // Merge the two pairs of half-edges
    targetNextTwin.twin = targetTwin
    targetTwin.twin = targetNextTwin

    // Assign the new edge
    targetTwin.edge = targetNextTwin.edge
    targetNextTwin.edge.halfedge = targetTwin

    // Update vertex connections
    vDelete.halfedge = vDelete.halfedge.next.twin
    v3.halfedge = targetTwin

    // Remove unnecessary elements
    erase_halfedge(targetH.next)
    erase_halfedge(targetH)
    erase_edge(eDelete)
    erase_face(fDelete)

    check_erase_face(f1, v0)
    check_erase_face(f2, v1)

    // Step 5: Remove old elements
    erase_edge(e)
    erase_halfedge(h)
    erase_halfedge(t)
    erase_vertex(v1)

    return v0  // Return the remaining vertex







function extrude_face(face f) :

    // Special Case: If the face is on the boundary, return null
    if f.is_boundary :
        return null

    // Step 1: Collect Data
    faceHalfedge_old = []
    faceVertices_old = []
    faceVertices_new = []

    h1 = f.halfedge
    do:
        faceHalfedge_old.append(h1)
        faceVertices_old.append(h1.vertex)
        h1 = h1.next
    while h1 != f.halfedge

    // Step 2: Create New Vertices (same positions as old ones)
    for v in faceVertices_old :
        newVertex = create_vertex()
        newVertex.position = v.position
        interpolate_data([v], newVertex)  // Copy UVs, normals, etc.
        faceVertices_new.append(newVertex)

    // Step 3: Create New Half-Edges for Inner Face
    twinHalfedges = []
    halfHalfedges = []
    for i in range(len(faceVertices_new)) :
        newTwin = create_halfedge()
        newHalf = create_halfedge()
        newTwin.vertex = faceVertices_new[i]
        newHalf.vertex = faceVertices_new[i]
        twinHalfedges.append(newTwin)
        halfHalfedges.append(newHalf)
        faceVertices_new[i].halfedge = newHalf

    // Step 4: Connect Inner Half-Edges and Create Edges
    for i in range(len(twinHalfedges)) :
        newEdge = create_edge()
        halfHalfedges[i].next = halfHalfedges[(i + 1) % len(halfHalfedges)]
        halfHalfedges[i].twin = twinHalfedges[(i + 1) % len(twinHalfedges)]
        halfHalfedges[i].edge = newEdge
        halfHalfedges[i].twin.edge = newEdge
        newEdge.halfedge = halfHalfedges[i]

        twinHalfedges[i].next = twinHalfedges[(i - 1) % len(twinHalfedges)]
        twinHalfedges[i].twin = halfHalfedges[(i - 1) % len(halfHalfedges)]

     // Step 5: Connect Old Half-Edges to New Half-Edges
     for i in range(len(twinHalfedges)) :
        newEdge = create_edge()
        h = create_halfedge()
        t = create_halfedge()
        h.edge = newEdge
        t.edge = newEdge
        h.twin = t
        t.twin = h
        newEdge.halfedge = h

        faceHalfedge_old[i].next = h
        h.next = twinHalfedges[(i + 1) % len(twinHalfedges)]
        h.vertex = faceHalfedge_old[(i + 1) % len(faceHalfedge_old)].vertex
        twinHalfedges[(i + 2) % len(twinHalfedges)].next = t
        t.next = faceHalfedge_old[(i + 1) % len(faceHalfedge_old)]
        t.vertex = faceVertices_new[(i + 1) % len(faceVertices_new)]

      // Step 6: Assign Faces
      assign_face_to_halfedges(halfHalfedges[0], f)
      for h in faceHalfedge_old :
        newFace = create_face()
        assign_face_to_halfedges(h, newFace)

      return f

