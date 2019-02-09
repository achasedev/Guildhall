/************************************************************************/
/* File: Chunk.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the Chunk class
/************************************************************************/
#include "Game/Environment/Chunk.hpp"
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
}


//-----------------------------------------------------------------------------------------------
// Populates the chunk with blocks using Perlin Noise
//
void Chunk::GenerateWithPerlinNoise(int seaLevel, int baseElevation, int maxDeviationFromBaseElevation)
{
	const BlockType* grassType = BlockType::GetTypeByName("Grass");
	const BlockType* stoneType = BlockType::GetTypeByName("Stone");

	m_meshBuilder.Clear();
	m_meshBuilder.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	for (int yIndex = 0; yIndex < CHUNK_DIMENSIONS_Y; ++yIndex)
	{
		for (int xIndex = 0; xIndex < CHUNK_DIMENSIONS_X; ++xIndex)
		{
			// Use XY center positions for the noise
			Vector2 blockXYCenter = Vector2(xIndex, yIndex) + Vector2(0.5f);

			// Get the height of the chunk at these coordinates
			float noise = Compute2dPerlinNoise(blockXYCenter.x, blockXYCenter.y);
			int xyHeight = RoundToNearestInt(noise * maxDeviationFromBaseElevation) + baseElevation;

			for (int zIndex = 0; zIndex < xyHeight; ++zIndex)
			{
				IntVector3 blockCoords = IntVector3(xIndex, yIndex, zIndex);

				const BlockType* typeToUse = nullptr;
				if (zIndex < seaLevel)
				{
					typeToUse = stoneType;
				}
				else
				{
					typeToUse = grassType;
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
	Material* material = AssetDB::CreateOrGetSharedMaterial("Default_Opaque");
	
	renderer->DrawMeshWithMaterial(m_mesh, material);
}


//-----------------------------------------------------------------------------------------------
// Pushes the vertices (and indices) into the meshbuilder for the block at the given coords
// PUSHES THE BLOCK VERTICES IN WORLD SPACE
//
void Chunk::PushVerticesForBlock(const IntVector3& blockCoords, const BlockType* type)
{
	int worldXOffset = m_chunkCoords.x * CHUNK_DIMENSIONS_X;
	int worldYOffset = m_chunkCoords.y * CHUNK_DIMENSIONS_Y;

	Vector3 cubeBottomSouthWest = Vector3(worldXOffset, worldYOffset, blockCoords.z);

	// Back (Facing -x direction)
	m_meshBuilder.Push3DQuad(cubeBottomSouthWest, Vector2::ONES, type->m_sideUVs, Rgba::WHITE, Vector3::MINUS_Y_AXIS, Vector3::Z_AXIS, Vector2::ZERO);
}
