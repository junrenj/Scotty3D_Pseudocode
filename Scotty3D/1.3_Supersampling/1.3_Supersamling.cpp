// This is just the pseudocode for Resolve Color
// It is a function to get color from your samples
function Resolve_Color
{
    // Create an empty image with the same resolution as the framebuffer
    image = HDR_Image(width, height)

    // Iterate through each pixel in the framebuffer
    for y from 0 to height - 1:
        for x from 0 to width - 1 :
            finalCol = Spectrum(0.0)  // Accumulate color values
            totalWeight = 0.0         // Sum of all sample weights

            // Iterate through all samples in the sample pattern
            for s from 0 to size(sample_pattern) - 1 :
                weight = sample_pattern[s].z  // Get sample weight
                finalCol += color_at(x, y, s) * weight            // Weighted color sum
                totalWeight += weight                             // Accumulate total weight

            // Normalize the accumulated color using the total weight
            finalCol = finalCol / totalWeight
            image.at(x, y) = finalCol  // Store the final resolved color

    return image  // Return the resolved image
}

// This is just the pseudocode for sample loop
function SampleLoop
{
	array <ClippedVertex> originalClipvertex
	array <ClippedVertex> supersampleVertex;
	// instead of move the sample pixel center, we move the clip vertex directly
	for samples from 0 to samples.size - 1:
		foreach clipVertex in originalClipvertex:
            // Make original Vertex offset
			offset = (samples[s].x - 0.5f, samples[s].y - 0.5f);
			supersampleVertex[j] = clipped_vertices[j];
			supersampleVertex[j].fb_position += offset;

        // Rasterize line or triangle according to mode
        if line mode
            Call rasterize_line function
        else if triangle mode
		    Call ratserize_triangle function

        // Get final color and blend together
        fb_depth = framebuffer.depth_at(x, y, s);
        fb_color = framebuffer.color_at(x, y, s);
}



// This is just the pseudocode for Sample pattern
function CreateYourOwnSamplePattern()->SamplePattern:
{
    // Unique ID for the custom sample pattern
    id = 0

    // Descriptive name for this custom pattern
    name = "Custom Sample Pattern"

    // Define sample points and their respective weights
    // This pattern is a Rotated Grid with four sample points:
    centers_and_weights = [
        Vec3(0.125, 0.625, 0.25),  // Sample point 1 with weight 0.25
        Vec3(0.375, 0.125, 0.25),  // Sample point 2 with weight 0.25
        Vec3(0.875, 0.375, 0.25),  // Sample point 3 with weight 0.25
        Vec3(0.625, 0.875, 0.25)   // Sample point 4 with weight 0.25
    ]

    // Return the constructed SamplePattern object
    RETURN SamplePattern(id, name, centers_and_weights)

}