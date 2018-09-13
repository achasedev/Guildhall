#version 430
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

// Set the work group dimensions
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; 
/*
#define MAX_VOXELS 128*128*64
#define MAX_CHUNKS 2048
#define MAX_VERTICES 8*8*8*6*4
#define MAX_INDICES 8*8*8*6*6

#define WORLD_X_DIMENSIONS 128
#define WORLD_Y_DIMENSIONS 64
#define WORLD_Z_DIMENSIONS 128
*/

#define MAX_VOXELS 8*8*8
#define MAX_CHUNKS 8
#define MAX_VERTICES 4*4*4*6*4
#define MAX_INDICES 4*4*4*6*6

#define WORLD_X_DIMENSIONS 8
#define WORLD_Y_DIMENSIONS 8
#define WORLD_Z_DIMENSIONS 8

struct VertexVoxel
{
	vec3 position;
	float color;
};

// Structs
struct ChunkData_t
{
	VertexVoxel		vertexBuffer[MAX_VERTICES];
	uint			indexBuffer[MAX_INDICES];

	uint vertexCount;
	uint indexCount;
	uint dualHead;
};

layout(binding=11, std140) buffer meshSSBO
{
	ChunkData_t chunkData[];
};

// Buffers
layout(binding=0, std140) uniform timeUBO
{
	float GAME_DELTA_TIME;
	float GAME_TOTAL_TIME;
	float SYSTEM_DELTA_TIME;
	float SYSTEM_TOTAL_TIME;
};

layout(binding=10, std140) buffer colorSSBO
{
	uint VOXEL_COLORS[MAX_VOXELS];
};	



bool IsNeighborEmpty(uvec3 currCoords, ivec3 direction)
{
	uvec3 neighborCoords = currCoords + direction;

	if (neighborCoords.x >= WORLD_X_DIMENSIONS || neighborCoords.y >= WORLD_Y_DIMENSIONS || neighborCoords.z >= WORLD_Z_DIMENSIONS)
	{
		return false;
	}

	uint neighborIndex = neighborCoords.y * (WORLD_X_DIMENSIONS * WORLD_Z_DIMENSIONS) + neighborCoords.z * WORLD_X_DIMENSIONS + neighborCoords.x;

	return ((VOXEL_COLORS[neighborIndex] & 255) > 0);
}

void main()
{
	uvec3 voxelCoords = gl_GlobalInvocationID;

	// Check the three neighbors
	bool addTop = IsNeighborEmpty(voxelCoords, ivec3(0,1,0));
	bool addLeft = IsNeighborEmpty(voxelCoords, ivec3(-1,0,0));
	bool addFront = IsNeighborEmpty(voxelCoords, ivec3(0,0,-1));

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

	//chunkData[chunkIndex].vertexCount += vertexCountToAdd;
	//chunkData[chunkIndex].indexCount += indexCountToAdd;
}