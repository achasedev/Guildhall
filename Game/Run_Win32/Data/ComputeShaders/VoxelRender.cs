#version 430

// Set the work group dimensions
layout(local_size_x = 1, local_size_y = 1) in; // Make each work group 1x1 items - a single pixel
layout(rgba8, binding = 0) uniform image2D image_output; // The image output - ensure the format matches the bound image!


void main()
{
	vec4 pixel = vec4(0.0, 0.0, 1.0, 1.0);

	// Get the index in the global work group
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	// Do the pixel work here

	// Output the color to the image
	imageStore(image_output, pixel_coords, pixel);
}

