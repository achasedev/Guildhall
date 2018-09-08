#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Set the work group dimensions
layout(local_size_x = 1, local_size_y = 1) in; // Make each work group 1x1 items - a single pixel
layout(rgba8, binding = 0) uniform image2D image_output; // The image output - ensure the format matches the bound image!

struct OctreeNode
{
	vec3 color;		// The color of the voxel
	float padding;
};

layout(binding=0, std140) buffer color_data
{
	ivec3 GRID_DIMENSIONS;
	int alsopadding;
	OctreeNode NODES[ ];
};	

void main()
{
	vec4 pixel = vec4(NODES[0].color, 1.0);
	//vec4 pixel = vec4(1.0, 0.0, 0.0, 1.0);

	// Get the index in the global work group
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

	// Do the pixel work here


	// Output the color to the image
	imageStore(image_output, pixel_coords, pixel);
}

