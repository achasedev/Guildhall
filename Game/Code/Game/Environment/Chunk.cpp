/************************************************************************/
/* File: Chunk.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the Chunk class
/************************************************************************/
#include "Game/Environment/Chunk.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Environment/BlockType.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/SmoothNoise.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
Chunk::Chunk(const IntVector2& chunkCoords)
	: m_chunkCoords(chunkCoords)
{
	// Set the world bounds
	m_worldBounds.mins = Vector3(chunkCoords.x * CHUNK_DIMENSIONS_X, chunkCoords.y * CHUNK_DIMENSIONS_Y, 0);
	m_worldBounds.maxs = m_worldBounds.mins + Vector3(CHUNK_DIMENSIONS_X, CHUNK_DIMENSIONS_Y, CHUNK_DIMENSIONS_Z);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Chunk::~Chunk()
{
	if (m_mesh != nullptr)
	{
		delete m_mesh;
		m_mesh = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Populates the chunk with blocks using Perlin Noise
//
void Chunk::GenerateWithPerlinNoise(int baseElevation, int maxDeviationFromBaseElevation)
{
	const BlockType* grassType = BlockType::GetTypeByName("Grass");
	const BlockType* dirtType = BlockType::GetTypeByName("Dirt");
	const BlockType* stoneType = BlockType::GetTypeByName("Stone");

	m_meshBuilder.Clear();
	m_meshBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	for (int yIndex = 0; yIndex < CHUNK_DIMENSIONS_Y; ++yIndex)
	{
		for (int xIndex = 0; xIndex < CHUNK_DIMENSIONS_X; ++xIndex)
		{
			// Use XY center positions for the noise, in world coordinates
			Vector2 chunkOffsetFromWorldOrigin = Vector2(m_chunkCoords.x * CHUNK_DIMENSIONS_X, m_chunkCoords.y * CHUNK_DIMENSIONS_Y);
			Vector2 blockXYCenter = chunkOffsetFromWorldOrigin + Vector2(xIndex, yIndex) + Vector2(0.5f);

			// Get the height of the chunk at these coordinates
			float noise = Compute2dPerlinNoise(blockXYCenter.x, blockXYCenter.y, 50.f);
			int grassHeight = RoundToNearestInt(noise * maxDeviationFromBaseElevation) + baseElevation;

			for (int zIndex = 0; zIndex < grassHeight; ++zIndex)
			{
				IntVector3 blockCoords = IntVector3(xIndex, yIndex, zIndex);

				const BlockType* typeToUse = nullptr;
				if (zIndex == grassHeight - 1)
				{
					typeToUse = grassType;
				}
				else if (zIndex >= grassHeight - 4) // Dirt for 3 blocks below the grass height
				{
					typeToUse = dirtType;
				}
				else
				{
					typeToUse = stoneType;
				}

				PushVerticesForBlock(blockCoords, typeToUse);
			}
		}
	}

	m_meshBuilder.FinishBuilding();

	if (m_mesh == nullptr)
	{
		m_mesh = m_meshBuilder.CreateMesh();
	}
	else
	{
		m_meshBuilder.UpdateMesh(*m_mesh);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Chunk::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Render
//
void Chunk::Render() const
{
	Renderer* renderer = Renderer::GetInstance();
	Material* material = AssetDB::CreateOrGetSharedMaterial("Data/Materials/Block.xml");
	
	renderer->DrawMeshWithMaterial(m_mesh, material);
}


//-----------------------------------------------------------------------------------------------
// Returns the XY center position of the chunk
//
Vector2 Chunk::GetWorldXYCenter() const
{
	float x = 0.5f * (m_worldBounds.mins.x + m_worldBounds.maxs.x);
	float y = 0.5f * (m_worldBounds.mins.y + m_worldBounds.maxs.y);

	return Vector2(x, y);
}


//-----------------------------------------------------------------------------------------------
// Returns the XY bounds of the chunk
//
AABB2 Chunk::GetWorldXYBounds() const
{
	AABB2 xyBounds;
	xyBounds.mins = m_worldBounds.mins.xy();
	xyBounds.maxs = m_worldBounds.maxs.xy();

	return xyBounds;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk coordinates of this chunk
//
IntVector2 Chunk::GetChunkCoords() const
{
	return m_chunkCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns the block index of the block given by blockCoords
//
int Chunk::GetBlockIndexFromBlockCoords(const IntVector3& blockCoords)
{
	int index = BLOCKS_PER_Z_LAYER * blockCoords.z + CHUNK_DIMENSIONS_X * blockCoords.y + blockCoords.x;
	return index;
}


//-----------------------------------------------------------------------------------------------
// Returns the block coords for the block given by blockIndex
//
IntVector3 Chunk::GetBlockCoordsFromBlockIndex(int blockIndex)
{
	int xMask = (CHUNK_DIMENSIONS_X - 1);
	int yMask = (CHUNK_DIMENSIONS_Y - 1);
	int zMask = (CHUNK_DIMENSIONS_Z - 1);

	int xCoord = blockIndex & xMask;
	int yCoord = (blockIndex >> CHUNK_BITS_X) & yMask;
	int zCoord = (blockIndex >> (CHUNK_BITS_X + CHUNK_BITS_Y)) & zMask;

	return IntVector3(xCoord, yCoord, zCoord);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices (and indices) into the meshbuilder for the block at the given coords
// PUSHES THE BLOCK VERTICES IN WORLD SPACE
//
void Chunk::PushVerticesForBlock(const IntVector3& blockCoords, const BlockType* type)
{
	int worldXOffset = m_chunkCoords.x * CHUNK_DIMENSIONS_X;
	int worldYOffset = m_chunkCoords.y * CHUNK_DIMENSIONS_Y;

	Vector3 cubeBottomSouthWest = Vector3(worldXOffset + blockCoords.x, worldYOffset + blockCoords.y, blockCoords.z);
	Vector3 cubeTopNorthEast = cubeBottomSouthWest + Vector3::ONES;

	// East Face
	m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::Z_AXIS, Vector2(1.0f, 0.f));

	// West Face
	m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::Y_AXIS, Vector3::Z_AXIS, Vector2(1.f, 1.0f));

	// North Face
	m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::MINUS_X_AXIS, Vector3::Z_AXIS, Vector2(0.f, 1.0f));

	// South Face
	m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::X_AXIS, Vector3::Z_AXIS, Vector2::ZERO);

	// Top Face
	m_meshBuilder.Push3DQuad(cubeTopNorthEast, Vector2::ONES, type->m_topUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::X_AXIS, Vector2(0.f, 1.0f));

	// Bottom Face
	m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_bottomUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::MINUS_X_AXIS, Vector2::ONES);
}
