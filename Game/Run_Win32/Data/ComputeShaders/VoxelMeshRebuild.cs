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

layout(binding=9, std430) buffer offsetSSBO
{
	uint OFFSETS[];
};	

layout(binding=10, std430) buffer vertexSSBO
{
	VertexVoxel VERTICES[];
};	

layout(binding=11, std430) buffer indexSSBO
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

void AddQuad(uvec3 coords, uint directionIndex, inout uint faceIndex)
{
	uint globalVertexOffset = faceIndex * 4;
	uint globalIndexOffset = faceIndex * 6;

	uvec3 globalDimensions = gl_NumWorkGroups * gl_WorkGroupSize;
	uint voxelIndex = coords.y * (globalDimensions.x * globalDimensions.z) + coords.z * globalDimensions.x + coords.x;

	// Push vertices
	for (int i = 0; i < 4; ++i)
	{
		VERTICES[globalVertexOffset + i].position = coords + g_vertexOffsets[directionIndex][i];
		//VERTICES[chunkVertexOffset + vertexOffset + i].position = vec3(vertexOffset + i);

		VERTICES[globalVertexOffset + i].color = VOXEL_COLORS[voxelIndex];
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
	bool shouldAddQuad[4]; // 6 directions, same order as g_neighborDirections
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