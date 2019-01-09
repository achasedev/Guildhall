#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Set the work group dimensions
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct VertexVoxel
{
	vec3 position;
	uint color;
};


// Buffers
layout(binding=0, std140) uniform timeUBO
{
	float GAME_DELTA_TIME;
	float GAME_TOTAL_TIME;
	float SYSTEM_DELTA_TIME;
	float SYSTEM_TOTAL_TIME;
};

layout(binding=8, std430) buffer colorSSBO
{
	uint VOXEL_COLORS[];
};	

layout(binding=9, std430) buffer metaSSBO
{
	uint VOXEL_META_DATA[];
};

layout(binding=10, std430) buffer offsetSSBO
{
	uint OFFSETS[];
};	

layout(binding=11, std430) buffer vertexSSBO
{
	VertexVoxel VERTICES[];
};	

layout(binding=12, std430) buffer indexSSBO
{
	uint INDICES[];
};	

//layout(binding=12, std140) buffer 
//{
//	ChunkMetaData_t CHUNK_DATA[];
//};	

// Globals
ivec3 g_neighborDirections[4] = ivec3[](
	ivec3(0,  0, -1),  // Front
	ivec3(-1, 0,  0),  // Left
	ivec3(1,  0,  0),  // Right
	ivec3(0,  1,  0)); // Top

vec3 g_vertexOffsets[4][4] = vec3[][](
	vec3[](vec3(0.0f, 0.0f, 0.0f), 	vec3(1.0f, 0.0f, 0.0f), 	vec3(1.0f, 1.0f, 0.0f), 	vec3(0.0f, 1.0f, 0.0f)),   	// Front
	vec3[](vec3(0.0f, 0.0f, 1.0f), 	vec3(0.0f, 0.0f, 0.0f), 	vec3(0.0f, 1.0f, 0.0f), 	vec3(0.0f, 1.0f, 1.0f)),   	// Left
	vec3[](vec3(1.0f, 0.0f, 0.0f), 	vec3(1.0f, 0.0f, 1.0f), 	vec3(1.0f, 1.0f, 1.0f), 	vec3(1.0f, 1.0f, 0.0f)),   	// Right
	vec3[](vec3(0.0f, 1.0f, 0.0f), 	vec3(1.0f, 1.0f, 0.0f), 	vec3(1.0f, 1.0f, 1.0f), 	vec3(0.0f, 1.0f, 1.0f))   	// Top
);

uint g_indexOffsets[6] = uint[](
	0, 1, 2,
	0, 2, 3);

float g_colorScalar[4] = float[](
	0.8f, 0.6f, 0.6f, 1.0f);

ivec2 g_shadowOffsets[4] = ivec2[](
	ivec2(0, -1),
	ivec2(-1, 0),
	ivec2(1, 0),
	ivec2(0, 0));


bool IsMetaBitSet(uint voxelIndex, int flagIndex)
{
	uint setIndex = voxelIndex / 4;
	uint subsetIndex = voxelIndex % 4;

	uint flagSet = VOXEL_META_DATA[setIndex];
	uint flags = (flagSet >> (8 * subsetIndex + flagIndex));

	return ((flags & 0x1) != 0);
}


bool IsNeighborEmpty(uvec3 currCoords, uint directionIndex)
{
	ivec3 direction = g_neighborDirections[directionIndex];
	ivec3 neighborCoords = ivec3(currCoords) + direction;

	uvec3 globalDimensions = gl_NumWorkGroups * gl_WorkGroupSize;

	bool edgeOfGrid = neighborCoords.x >= globalDimensions.x || neighborCoords.y >= globalDimensions.y || neighborCoords.z >= globalDimensions.z || neighborCoords.x < 0 || neighborCoords.y < 0 || neighborCoords.z < 0;
	if (edgeOfGrid)
	{
		return true;
	}

	uint neighborIndex = neighborCoords.y * (globalDimensions.x * globalDimensions.z) + neighborCoords.z * globalDimensions.x + neighborCoords.x;

	return ((VOXEL_COLORS[neighborIndex] & 0xff000000) == 0);
}

vec4 FromUInt(uint intColor)
{
	float red = float(intColor & 0xff) / 255.0f;
	float green = float((intColor & 0xff00) >> 8) / 255.0f;
	float blue = float((intColor & 0xff0000) >> 16) / 255.0f;
	float alpha = float((intColor & 0xff000000) >> 24) / 255.0f;

	return vec4(red, green, blue, alpha);

}

uint ToUInt(vec4 floatColor)
{
	floatColor = clamp(floatColor, vec4(0.0f), vec4(1.0f));

	uint red = uint(floatColor.x * 255.0f);
	uint green = uint(floatColor.y * 255.0f);
	uint blue = uint(floatColor.z * 255.0f);
	uint alpha = uint(floatColor.w * 255.0f);

	green = green << 8;
	blue = blue << 16;
	alpha = alpha << 24;

	return (red | green | blue | alpha);
}


float GetShadowScalar(uvec3 coords, uint directionIndex)
{
	// First check if we should even receive a shadow
	uvec3 globalDimensions = gl_NumWorkGroups * gl_WorkGroupSize;
	uint baseIndex = coords.y * (globalDimensions.x * globalDimensions.z) + coords.z * globalDimensions.x + coords.x;
 
 	if (!IsMetaBitSet(baseIndex, 1))
 	{
 		return 1.0f;
 	}

	// Check the column we're in, if anything is above us then we're completely shadowed
	// (Direct overhead light)

	ivec2 offsets = g_shadowOffsets[directionIndex];
	ivec2 xzCoordsPreCheck = ivec2(coords.xz) + offsets;

	// Edge case checks
	if (xzCoordsPreCheck.x < 0 || xzCoordsPreCheck.x >= globalDimensions.x || xzCoordsPreCheck.y < 0 || xzCoordsPreCheck.y >= globalDimensions.z)
	{
		return 1.0f; // Don't shadow faces that are on the edge
	}

	uvec2 xzCoords = uvec2(xzCoordsPreCheck);

	for (uint yCoord = coords.y + 1; yCoord < globalDimensions.y; ++yCoord)
	{
		uint voxelIndex = yCoord * (globalDimensions.x * globalDimensions.z) + xzCoords.y * globalDimensions.x + xzCoords.x;

		// If voxel above is solid and casts shadows, then we return a darkening scalar
		if ((VOXEL_COLORS[voxelIndex] & 0xff000000) != 0 && (IsMetaBitSet(voxelIndex, 0)))
		{
			return 0.5f;
		}
	}

	return 1.0f; // No shadow
}


void AddQuad(uvec3 coords, uint directionIndex, inout uint faceIndex)
{
	uint globalVertexOffset = faceIndex * 4;
	uint globalIndexOffset = faceIndex * 6;

	uvec3 globalDimensions = gl_NumWorkGroups * gl_WorkGroupSize;
	uint voxelIndex = coords.y * (globalDimensions.x * globalDimensions.z) + coords.z * globalDimensions.x + coords.x;

	// Calculate color
	uint baseColor = VOXEL_COLORS[voxelIndex];
	vec4 floatColor = FromUInt(baseColor) * g_colorScalar[directionIndex];

	// Applies the shadowing
	floatColor *= GetShadowScalar(coords, directionIndex);

	uint finalColor = ToUInt(floatColor);

	// Push vertices
	for (int i = 0; i < 4; ++i)
	{
		VERTICES[globalVertexOffset + i].position = coords + g_vertexOffsets[directionIndex][i];
		//VERTICES[chunkVertexOffset + vertexOffset + i].position = vec3(vertexOffset + i);

		VERTICES[globalVertexOffset + i].color = finalColor;
	}

	// Push Indices
	for (int i = 0; i < 6; ++i)
	{
		INDICES[globalIndexOffset + i] = globalVertexOffset + g_indexOffsets[i];
	}

	faceIndex++;
}

void main()
{	
	uvec3 voxelCoords = gl_GlobalInvocationID;
	uvec3 globalDimensions = gl_NumWorkGroups * gl_WorkGroupSize;

	uint voxelIndex = voxelCoords.y * (globalDimensions.x * globalDimensions.z) + voxelCoords.z * globalDimensions.x + voxelCoords.x;

	// If we're non-solid don't do anything
	if ((VOXEL_COLORS[voxelIndex] & 0xff000000) == 0)
	{
		return;
	}

	// Check the neighbors
	bool shouldAddQuad[4]; // 4 directions, same order as g_neighborDirections
	int sideCountToAdd = 0;

	// Figure out which sides we have to add
	for (uint i = 0; i < 4; ++i)
	{
		shouldAddQuad[i] = IsNeighborEmpty(voxelCoords, i);

		if (shouldAddQuad[i])
		{
			sideCountToAdd++;
		}
	}

	uint writeHead = atomicAdd(OFFSETS[0], sideCountToAdd);

	// Add the sides
	for (int i = 0; i < 4; ++i)
	{
		if (shouldAddQuad[i])
		{
			AddQuad(voxelCoords, i, writeHead);
		}
	}


	// Done!
}