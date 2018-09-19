/************************************************************************/
/* File: VoxelGrid.cpp
/* Author: Andrew Chase
/* Date: September 15th, 2018
/* Description: Implementation of the VoxelGrid class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileLogScoped.hpp"
#include "Engine/Rendering/Resources/Texture3D.hpp"
#include "Engine/Rendering/Shaders/ComputeShader.hpp"

// Constants for any size grid
#define VERTICES_PER_VOXEL 24
#define INDICES_PER_VOXEL 36

#define COLOR_BINDING (8)
#define COUNT_BINDING (9)
#define VERTEX_BINDING (10)
#define INDEX_BINDING (11)


//-----------------------------------------------------------------------------------------------
// Initializes the grid buffers and dimensions
//
void VoxelGrid::Initialize(const IntVector3& voxelDimensions)
{
	m_dimensions = voxelDimensions;

	int numVoxels = m_dimensions.x * m_dimensions.y * m_dimensions.z;

	m_gridColors = (Rgba*)malloc(numVoxels * sizeof(Rgba));
	memset(m_gridColors, 0, numVoxels * sizeof(Rgba));

// 	for (int i = 0; i < numVoxels; ++i)
// 	{
// 		m_gridColors[i] = Rgba::GetRandomColor();
// 		IntVector3 coords = GetCoordsForIndex(i);
// 
// 		if ((coords.x + coords.y + coords.z) % 2 == 1)
// 		{
// 			m_gridColors[i].a = 0;
// 		}
// 	}

	// Initialize mesh building steps
	m_computeShader = new ComputeShader();
	m_computeShader->Initialize("Data/ComputeShaders/VoxelMeshRebuild.cs");

	InitializeBuffers();
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh for the grid and draws the mesh to screen
//
void VoxelGrid::BuildMeshAndDraw()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Rebuild the mesh
	RebuildMesh();

	// Draw the mesh
	DrawGrid();
}


//-----------------------------------------------------------------------------------------------
// Draws the 3D texture to the grid
//
void VoxelGrid::Write3DTexture(const Vector3& position, float rotation, Texture3D* texture)
{
	IntVector3 dimensions = texture->GetDimensions();
	IntVector3 halfDimensions = dimensions / 2;

	// Position the object occupies
	IntVector3 coordinatePosition = IntVector3(position.x, position.y, position.z);

	IntVector3 bottomLeft = coordinatePosition - halfDimensions;

	for (int xOff = 0; xOff < dimensions.x; ++xOff)
	{
		for (int yOff = 0; yOff < dimensions.y; ++yOff)
		{
			for (int zOff = 0; zOff < dimensions.z; ++zOff)
			{
				IntVector3 localCoords = IntVector3(xOff, yOff, zOff);
				IntVector3 currCoords = bottomLeft + localCoords;

				int index = GetIndexForCoords(currCoords);
				m_gridColors[index] = texture->GetColorAtCoords(localCoords);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the number of voxels in this grid (solid and non-solid)
//
int VoxelGrid::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}


//-----------------------------------------------------------------------------------------------
// Returns the linear index for the voxel given by coords
//
int VoxelGrid::GetIndexForCoords(const IntVector3& coords) const
{
	return coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
}


//-----------------------------------------------------------------------------------------------
// Returns the coordinates in the grid for the voxel given by index
//
IntVector3 VoxelGrid::GetCoordsForIndex(unsigned int index) const
{
	int y = index / (m_dimensions.x * m_dimensions.z);
	int leftover = index % (m_dimensions.x * m_dimensions.z);

	int z = leftover / m_dimensions.x;
	int x = leftover % m_dimensions.x;

	return IntVector3(x, y, z);
}


//-----------------------------------------------------------------------------------------------
// Binds and initializes the buffer data for the buffers used for GPU operations
//
void VoxelGrid::InitializeBuffers()
{
	int voxelCount = GetVoxelCount();

	// Color Buffer
	m_colorBuffer.Bind(COLOR_BINDING);
	m_colorBuffer.CopyToGPU(voxelCount * sizeof(Rgba), nullptr);

	// Count Buffer
	m_countBuffer.Bind(COUNT_BINDING);

	int val = 0;
	m_countBuffer.CopyToGPU(sizeof(unsigned int), &val); // Initialize it to 0 so compute can start incrementing

	unsigned int vertexCount = voxelCount * VERTICES_PER_VOXEL;
	unsigned int indexCount = voxelCount * INDICES_PER_VOXEL;

	// Setup the mesh so the compute shader can directly write to its buffers
	m_mesh.InitializeBuffersForCompute<VertexVoxel>((unsigned int)VERTEX_BINDING, vertexCount, (unsigned int)INDEX_BINDING, indexCount);
}


//-----------------------------------------------------------------------------------------------
// Updates the GPU-side buffers in preparation for the next build
//
void VoxelGrid::UpdateBuffers()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	unsigned int voxelCount = GetVoxelCount();
	//memset(m_gridColors, 0, voxelCount * sizeof(Rgba));

	// Send down the color data
	m_colorBuffer.CopyToGPU(GetVoxelCount() * sizeof(Rgba), m_gridColors);

	// Clear the face count
	unsigned int val = 0;
	m_countBuffer.CopyToGPU(sizeof(unsigned int), &val);
}


//-----------------------------------------------------------------------------------------------
// Initializes the grid buffers and dimensions
//
void VoxelGrid::RebuildMesh()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Update the GPU-side buffers
	UpdateBuffers();

	// Execute the build step
	m_computeShader->Execute(m_dimensions.x / 8, m_dimensions.y / 8, m_dimensions.z / 8);

	// Get the vertex and index count from the buffer
	unsigned int* offset = (unsigned int*) m_countBuffer.MapBufferData();
	unsigned int faceOffset = offset[0];
	m_countBuffer.UnmapBufferData();

	// Get the counts
	unsigned int vertexCount = faceOffset * 4;
	unsigned int indexCount = faceOffset * 6;

	// Update the mesh's CPU side data
	m_mesh.UpdateCounts(vertexCount, indexCount);
	m_mesh.SetDrawInstruction(PRIMITIVE_TRIANGLES, true, 0, indexCount);
}


//-----------------------------------------------------------------------------------------------
// Draws the grid mesh to the screen
//
void VoxelGrid::DrawGrid()
{
	PROFILE_LOG_SCOPE_FUNCTION();

	// Draw
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(Game::GetGameCamera());

	Renderable rend;
	rend.AddInstanceMatrix(Matrix44::IDENTITY);

	RenderableDraw_t draw;
	draw.mesh = &m_mesh;
	draw.sharedMaterial = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");

	rend.AddDraw(draw);

	renderer->DrawRenderable(&rend);
}
