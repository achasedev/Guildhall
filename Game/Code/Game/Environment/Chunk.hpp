/************************************************************************/
/* File: Chunk.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent a vertical slice of the world
/************************************************************************/
#pragma once
#include "Game/Environment/Block.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"

class Mesh;
class BlockType;

class Chunk
{
public:
	//-----Public Methods-----

	Chunk(const IntVector2& chunkCoords);
	~Chunk();

	void GenerateWithPerlinNoise(int baseElevation, int maxDeviationFromBaseElevation);

	void Update();
	void Render() const;

	// Producers
	Vector2 GetWorldXYCenter() const;
	AABB2	GetWorldXYBounds() const;

	// Accessors
	IntVector2 GetChunkCoords() const;

	// Mutators

	// Statics
	static int			GetBlockIndexFromBlockCoords(const IntVector3& blockCoords);
	static IntVector3	GetBlockCoordsFromBlockIndex(int blockIndex);


public:
	//-----Public Data-----

	static constexpr int CHUNK_BITS_X = 4; // Number of bits in the Block Index to represent the x index
	static constexpr int CHUNK_BITS_Y = 4; // Number of bits in the Block Index to represent the y index
	static constexpr int CHUNK_BITS_Z = 8; // Number of bits in the Block Index to represent the z index

	static constexpr int CHUNK_DIMENSIONS_X = (1 << CHUNK_BITS_X);
	static constexpr int CHUNK_DIMENSIONS_Y = (1 << CHUNK_BITS_Y);
	static constexpr int CHUNK_DIMENSIONS_Z = (1 << CHUNK_BITS_Z);

	static constexpr int BLOCKS_PER_Z_LAYER = CHUNK_DIMENSIONS_X * CHUNK_DIMENSIONS_Y;
	static constexpr int BLOCKS_PER_CHUNK = CHUNK_DIMENSIONS_X * CHUNK_DIMENSIONS_Y * CHUNK_DIMENSIONS_Z;


private:
	//-----Private Methods------

	void PushVerticesForBlock(const IntVector3& blockCoords, const BlockType* type);


private:
	//-----Private Data-----

	IntVector2	m_chunkCoords;
	AABB3		m_worldBounds;
	Block		m_blocks[BLOCKS_PER_CHUNK]; // Blocks are index over x first, then y, then z
	
	Mesh*		m_mesh = nullptr;
	MeshBuilder m_meshBuilder;

};
