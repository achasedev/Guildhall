#include "Game/Framework/VoxelGridBuffers.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/EngineCommon.hpp"

#define COLOR_BINDING (8)
#define OFFSET_BINDING (9)
#define VERTEX_BINDING (10)
#define INDEX_BINDING (11)

void VoxelGridBuffers::Initialize(const IntVector3& worldDimensions, const IntVector3& chunkDimensions)
{
	int voxelCount = worldDimensions.x * worldDimensions.y * worldDimensions.z;
	int chunkCount = (worldDimensions.x / chunkDimensions.x) * (worldDimensions.y / chunkDimensions.y) * (worldDimensions.z / chunkDimensions.z);

	int voxelsPerChunk = chunkDimensions.x * chunkDimensions.y * chunkDimensions.z;
	int verticesPerVoxel = 6 * 4; // 6 faces, 4 vertices per face
	int indicesPerVoxel = 6 * 6; // 6 faces, each with 6 indices for the 2 triangles

	ASSERT_OR_DIE(voxelCount == chunkCount * voxelsPerChunk, "Math didn't add up when initializing");

	// Color Buffer
	m_colorBuffer.Bind(COLOR_BINDING);
	//m_colorBuffer.CopyToGPU(voxelCount, NULL);
	m_colorBuffer.CopyToGPU(voxelCount * 4, nullptr);

	// Offset Buffer
	m_offsetBuffer.Bind(OFFSET_BINDING);
	//m_offsetBuffer.CopyToGPU(sizeof(unsigned int) * chunkCount, NULL);
	int val = 0;
	m_offsetBuffer.CopyToGPU(sizeof(unsigned int), &val);

	unsigned int vertexCount = chunkCount * voxelsPerChunk * verticesPerVoxel;
	unsigned int indexCount = chunkCount * voxelsPerChunk * indicesPerVoxel;

	m_mesh.InitializeBuffersForCompute<VertexVoxel>((unsigned int)VERTEX_BINDING, vertexCount, (unsigned int)INDEX_BINDING, indexCount);

	// Vertex Buffer
// 	m_mesh.GetVertexBuffer()->Bind(VERTEX_BINDING);
// 	//m_vertexBuffer.Bind(VERTEX_BINDING);
// 	//m_vertexBuffer.CopyToGPU(chunkCount * voxelsPerChunk * verticesPerVoxel * sizeof(VertexVoxel), NULL);
// 	m_mesh.GetVertexBuffer()->CopyToGPU<VertexVoxel>(vertexCount, nullptr);
// 
// 	// Index Buffer
// 	m_mesh.GetIndexBuffer()->Bind(INDEX_BINDING);
// 	//m_indexBuffer.CopyToGPU(chunkCount * voxelsPerChunk * indicesPerVoxel * sizeof(unsigned int), NULL);
// 	m_mesh.GetIndexBuffer()->CopyToGPU(, nullptr);
}
