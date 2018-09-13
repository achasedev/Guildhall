#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Set the work group dimensions
layout(local_size_variable) in; 

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

layout(binding=8, std140) buffer colorSSBO
{
	uint VOXEL_COLORS[];
};	

layout(binding=9, std140) buffer offsetSSBO
{
	uint OFFSETS[];
};	

layout(binding=10, std140) buffer vertexSSBO
{
	VoxelVertex VERTICES[];
};	

layout(binding=11, std140) buffer indexSSBO
{
	uint INDICES[];
};	

//layout(binding=12, std140) buffer 
//{
//	ChunkMetaData_t CHUNK_DATA[];
//};	

// Globals
ivec3 g_neighborDirections[6] = vec3[](
	ivec3(0,  0, -1),  // Front
	ivec3(0,  0,  1),  // Back
	ivec3(-1, 0,  0),  // Left
	ivec3(1,  0,  0),  // Right
	ivec3(0,  1,  0),  // Top
	ivec3(0, -1,  0)); // Bottom

vec3 g_vertexOffsets[6][4] = vec3[][](
	vec3[](vec3(-0.5f, -0.5f, -0.5f), 	vec3( 0.5f, -0.5f, -0.5f), 	vec3( 0.5f,  0.5f, -0.5f), 	vec3(-0.5f,  0.5f, -0.5)),   	// Front
	vec3[](vec3( 0.5f, -0.5f,  0.5f), 	vec3(-0.5f, -0.5f,  0.5f), 	vec3(-0.5f,  0.5f,  0.5f), 	vec3( 0.5f,  0.5f,  0.5)),   	// Back
	vec3[](vec3(-0.5f, -0.5f,  0.5f), 	vec3(-0.5f, -0.5f,  0.5f), 	vec3(-0.5f,  0.5f, -0.5f), 	vec3(-0.5f,  0.5f,  0.5)),   	// Left
	vec3[](vec3( 0.5f, -0.5f, -0.5f), 	vec3( 0.5f, -0.5f,  0.5f), 	vec3( 0.5f,  0.5f,  0.5f), 	vec3( 0.5f,  0.5f, -0.5)),   	// Right
	vec3[](vec3(-0.5f,  0.5f, -0.5f), 	vec3( 0.5f,  0.5f, -0.5f), 	vec3( 0.5f,  0.5f,  0.5f), 	vec3(-0.5f,  0.5f,  0.5)),   	// Top
	vec3[](vec3(-0.5f, -0.5f,  0.5f), 	vec3( 0.5f, -0.5f,  0.5f), 	vec3( 0.5f, -0.5f, -0.5f), 	vec3(-0.5f, -0.5f, -0.5))); 	// Bottom

bool IsNeighborEmpty(ivec3 currCoords, int directionIndex)
{
	ivec3 direction = g_neighborDirections[directionIndex];
	ivec3 neighborCoords = currCoords + direction;

	if (neighborCoords.x >= WORLD_X_DIMENSIONS || neighborCoords.y >= WORLD_Y_DIMENSIONS || neighborCoords.z >= WORLD_Z_DIMENSIONS)
	{
		return false;
	}

	uint neighborIndex = neighborCoords.y * (WORLD_X_DIMENSIONS * WORLD_Z_DIMENSIONS) + neighborCoords.z * WORLD_X_DIMENSIONS + neighborCoords.x;

	return ((VOXEL_COLORS[neighborIndex] & 255) > 0);
}

void AddQuad(ivec3 coords, int directionIndex)
{
	//TODO: THIS
}

void main()
{
	ivec3 voxelCoords = gl_GlobalInvocationID;

	// Check the neighbors
	bool shouldAddQuad[6]; // 6 directions, same order as g_neighborDirections

	// Figure out which sides we have to add
	for (int i = 0; i < 6; ++i)
	{
		shouldAddQuad[i] = IsNeighborEmpty(voxelCoords, i);
	}

	// Add the sides
	for (int i = 0; i < 6; ++i)
	{
		if (shouldAddQuad[i])
		{
			AddQuad(ivec3 voxelCoords, int directionIndex);
		}
	}



	int numFacesToAdd = 0;
	if (addTop)
	{
		numFacesToAdd += 1;
	}
		
	if (addLeft)
	{
		numFacesToAdd += 1;
	}

	if (addFront)
	{
		numFacesToAdd += 1;
	}

	int vertexCountToAdd = numFacesToAdd * 4;
	int indexCountToAdd = numFacesToAdd * 6;

	uint offset = (vertexCountToAdd << 16) | indexCountToAdd;


	uvec3 chunkCoords = gl_WorkGroupID;
	uint chunkIndex = chunkCoords.y * (gl_NumWorkGroups.x * gl_NumWorkGroups.z) + chunkCoords.z * gl_NumWorkGroups.x + chunkCoords.x;

	uint currHead = atomicAdd(chunkData[chunkIndex].dualHead, offset);

	uint indexOffset = currHead & 65535;
	uint vertexOffset = currHead >> 16;

	uint voxelIndex = voxelCoords.y * (WORLD_X_DIMENSIONS * WORLD_Z_DIMENSIONS) + voxelCoords.z * WORLD_X_DIMENSIONS + voxelCoords.x;

	if (addTop)
	{
		// Front left
		chunkData[chunkIndex].vertexBuffer[vertexOffset].position = voxelCoords + vec3(-0.5, 0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset].color = VOXEL_COLORS[voxelIndex];

		// Front right
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].position = voxelCoords + vec3(0.5, 0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].color = VOXEL_COLORS[voxelIndex];

		// Back right
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].position = voxelCoords + vec3(0.5, 0.5, 0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].color = VOXEL_COLORS[voxelIndex];

		// Back left
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].position = voxelCoords + vec3(-0.5, 0.5, 0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].color = VOXEL_COLORS[voxelIndex];

		// Indices
		chunkData[chunkIndex].indexBuffer[indexOffset] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 1] = vertexOffset + 1;
		chunkData[chunkIndex].indexBuffer[indexOffset + 2] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 3] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 4] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 5] = vertexOffset + 3;

		vertexOffset += 4;
		indexOffset += 6;

	}
/*
	if (addLeft)
	{
		// Bottom back
		chunkData[chunkIndex].vertexBuffer[vertexOffset].position = voxelCoords + vec3(-0.5, -0.5, 0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset].color = VOXEL_COLORS[voxelIndex];

		// Bottom front
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].position = voxelCoords + vec3(-0.5, -0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].color = VOXEL_COLORS[voxelIndex];

		// Top front
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].position = voxelCoords + vec3(-0.5, 0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].color = VOXEL_COLORS[voxelIndex];

		// Top back
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].position = voxelCoords + vec3(-0.5, 0.5, 0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].color = VOXEL_COLORS[voxelIndex];

		// Indices
		chunkData[chunkIndex].indexBuffer[indexOffset] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 1] = vertexOffset + 1;
		chunkData[chunkIndex].indexBuffer[indexOffset + 2] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 3] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 4] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 5] = vertexOffset + 3;

		vertexOffset += 4;
		indexOffset += 6;
	}

	if (addFront)
	{
		// Bottom left
		chunkData[chunkIndex].vertexBuffer[vertexOffset].position = voxelCoords + vec3(-0.5, -0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset].color = VOXEL_COLORS[voxelIndex];

		// Bottom right
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].position = voxelCoords + vec3(0.5, -0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 1].color = VOXEL_COLORS[voxelIndex];

		// Top right
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].position = voxelCoords + vec3(0.5, 0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 2].color = VOXEL_COLORS[voxelIndex];

		// Top left
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].position = voxelCoords + vec3(-0.5, 0.5, -0.5);
		chunkData[chunkIndex].vertexBuffer[vertexOffset + 3].color = VOXEL_COLORS[voxelIndex];

		// Indices
		chunkData[chunkIndex].indexBuffer[indexOffset] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 1] = vertexOffset + 1;
		chunkData[chunkIndex].indexBuffer[indexOffset + 2] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 3] = vertexOffset;
		chunkData[chunkIndex].indexBuffer[indexOffset + 4] = vertexOffset + 2;
		chunkData[chunkIndex].indexBuffer[indexOffset + 5] = vertexOffset + 3;

		vertexOffset += 4;
		indexOffset += 6;
	}
*/
	//chunkData[chunkIndex].vertexCount += vertexCountToAdd;
	//chunkData[chunkIndex].indexCount += indexCountToAdd;
}