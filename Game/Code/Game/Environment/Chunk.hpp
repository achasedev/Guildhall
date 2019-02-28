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

class File;
class Mesh;
class BlockType;
class BlockLocator;

class Chunk
{
public:
	//-----Public Methods-----

	Chunk(const IntVector2& chunkCoords);
	~Chunk();

	bool					InitializeFromFile(const std::string& filepath);
	void					GenerateWithPerlinNoise(int baseElevation, int maxDeviationFromBaseElevation, int seaLevel);
	void					BuildMesh();

	void					Update();
	void					Render() const;

	// Producers
	Vector2					GetWorldXYCenter() const;
	AABB2					GetWorldXYBounds() const;
	Vector3					GetOriginWorldPosition() const;
	BlockLocator			GetBlockLocatorThatContainsPosition(const Vector3& worldPosition);
	void					WriteToFile() const;

	// Accessors
	inline Block&			GetBlock(int blockIndex);
	inline Block&			GetBlock(const IntVector3& blockCoords);
	inline BlockLocator		GetBlockLocator(int blockIndex);
	inline BlockLocator		GetBlockLocator(const IntVector3& blockCoords); 
	inline IntVector2		GetChunkCoords() const;
	inline bool				ShouldWriteToFile() const;
	inline Chunk*			GetEastNeighbor() const;
	inline Chunk*			GetWestNeighbor() const;
	inline Chunk*			GetNorthNeighbor() const;
	inline Chunk*			GetSouthNeighbor() const;
	inline bool				HasAllFourNeighbors() const;
	inline bool				IsMeshDirty() const;

	// Mutators
	inline void				SetEastNeighbor(Chunk* chunkToEast);
	inline void				SetWestNeighbor(Chunk* chunkToWest);
	inline void				SetNorthNeighbor(Chunk* chunkToNorth);
	inline void				SetSouthNeighbor(Chunk* chunkToSouth);
	inline void				SetIsMeshDirty(bool isMeshDirty);
	inline void				SetNeedsToBeSavedToDisk(bool needsToBeSaved);
	inline void				SetBlockTypeAtBlockIndex(int blockIndex, const BlockType* blockType);
	inline void				SetBlockTypeAtBlockCoords(const IntVector3& blockCoords, const BlockType* blockType);

	// Statics
	static int				GetBlockIndexFromBlockCoords(const IntVector3& blockCoords);
	static IntVector3		GetBlockCoordsFromBlockIndex(int blockIndex);


public:
	//-----Public Data-----

	static constexpr uint8_t	CHUNK_VERSION = 1;

	static constexpr uint8_t	CHUNK_BITS_X = 4;									// Number of bits in the Block Index to represent the x coordinate
	static constexpr uint8_t	CHUNK_BITS_Y = 4;									// Number of bits in the Block Index to represent the y coordinate
	static constexpr uint8_t	CHUNK_BITS_XY = CHUNK_BITS_X + CHUNK_BITS_Y;		// Number of bits in the Block Index for both x and y
	static constexpr uint8_t	CHUNK_BITS_Z = 8;									// Number of bits in the Block Index to represent the z coordinate

	static constexpr int		CHUNK_DIMENSIONS_X = (1 << CHUNK_BITS_X);
	static constexpr int		CHUNK_DIMENSIONS_Y = (1 << CHUNK_BITS_Y);
	static constexpr int		CHUNK_DIMENSIONS_Z = (1 << CHUNK_BITS_Z);

	static constexpr int		CHUNK_X_MASK = CHUNK_DIMENSIONS_X - 1;						// Mask that when applied to a BlockIndex will give the block's X coordinate
	static constexpr int		CHUNK_Y_MASK = (CHUNK_DIMENSIONS_Y - 1) << CHUNK_BITS_X;	// Mask that when applied to a BlockIndex will give the block's Y coordinate
	static constexpr int		CHUNK_Z_MASK = (CHUNK_DIMENSIONS_Z - 1) << CHUNK_BITS_XY;	// Mask that when applied to a BlockIndex will give the block's Z coordinate

	static constexpr int		BLOCKS_PER_Z_LAYER = CHUNK_DIMENSIONS_X * CHUNK_DIMENSIONS_Y;
	static constexpr int		BLOCKS_PER_CHUNK = CHUNK_DIMENSIONS_X * CHUNK_DIMENSIONS_Y * CHUNK_DIMENSIONS_Z;


private:
	//-----Private Methods------

	void PushVerticesForBlock(const IntVector3& blockCoords, const BlockType* type);
	void PushVerticesForBlock(int blockIndex, const BlockType* type);


private:
	//-----Private Data-----

	IntVector2	m_chunkCoords;
	AABB3		m_worldBounds;
	Block		m_blocks[BLOCKS_PER_CHUNK]; // Blocks are index over x first, then y, then z
	
	// For hidden surface removal and iterating over blocks
	Chunk*		m_eastNeighborChunk = nullptr;
	Chunk*		m_westNeighborChunk = nullptr;
	Chunk*		m_northNeighborChunk = nullptr;
	Chunk*		m_southNeighborChunk = nullptr;

	Mesh*		m_mesh = nullptr;
	MeshBuilder m_meshBuilder;
	
	bool		m_isMeshDirty = true;
	bool		m_needsToBeSaved = false;

};
