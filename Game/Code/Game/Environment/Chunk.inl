/************************************************************************/
/* File: Chunk.inl
/* Author: Andrew Chase
/* Date: February 28th 2019
/* Description: Inline functions for Chunk class
/************************************************************************/
#include "Game/Environment/Block.hpp"
#include "Game/Environment/Chunk.hpp"
#include "Game/Environment/BlockLocator.hpp"


//-----------------------------------------------------------------------------------------------
// Returns the block at the given block index
//
inline Block& Chunk::GetBlock(int blockIndex)
{
	return m_blocks[blockIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the block at the given coords
//
inline Block& Chunk::GetBlock(const IntVector3& blockCoords)
{
	int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);
	return GetBlock(blockIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk coordinates of this chunk
//
inline IntVector2 Chunk::GetChunkCoords() const
{
	return m_chunkCoords;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this chunk needs to be written to file when deactivated
//
inline bool Chunk::ShouldWriteToFile() const
{
	return m_needsToBeSaved;
}

//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the east of this chunk
//
inline Chunk* Chunk::GetEastNeighbor() const
{
	return m_eastNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the west of this chunk
//
inline Chunk* Chunk::GetWestNeighbor() const
{
	return m_westNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the north of this chunk
//
inline Chunk* Chunk::GetNorthNeighbor() const
{
	return m_northNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk directly to the south of this chunk
//
inline Chunk* Chunk::GetSouthNeighbor() const
{
	return m_southNeighborChunk;
}


//-----------------------------------------------------------------------------------------------
// Returns true if all neighbors to this chunk exist and are in the world (may not have meshes)
//
inline bool Chunk::HasAllFourNeighbors() const
{
	return (m_eastNeighborChunk != nullptr && m_westNeighborChunk != nullptr
		&& m_northNeighborChunk != nullptr && m_southNeighborChunk != nullptr);
}


//-----------------------------------------------------------------------------------------------
// Returns whether this chunk's mesh needs to be (re)built
//
inline bool Chunk::IsMeshDirty() const
{
	return m_isMeshDirty;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's east neighbor to the one provided
//
inline void Chunk::SetEastNeighbor(Chunk* chunkToEast)
{
	m_eastNeighborChunk = chunkToEast;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's west neighbor to the one provided
//
inline void Chunk::SetWestNeighbor(Chunk* chunkToWest)
{
	m_westNeighborChunk = chunkToWest;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's north neighbor to the one provided
//
inline void Chunk::SetNorthNeighbor(Chunk* chunkToNorth)
{
	m_northNeighborChunk = chunkToNorth;
}


//-----------------------------------------------------------------------------------------------
// Sets this chunk's south neighbor to the one provided
//
inline void Chunk::SetSouthNeighbor(Chunk* chunkToSouth)
{
	m_southNeighborChunk = chunkToSouth;
}


//-----------------------------------------------------------------------------------------------
// Sets whether the mesh of this chunk is dirty or not
//
inline void Chunk::SetIsMeshDirty(bool isMeshDirty)
{
	m_isMeshDirty = isMeshDirty;
}


//-----------------------------------------------------------------------------------------------
// Sets whether this chunk needs to be written to disk when deactivated
//
inline void Chunk::SetNeedsToBeSavedToDisk(bool needsToBeSaved)
{
	m_needsToBeSaved = needsToBeSaved;
}


//-----------------------------------------------------------------------------------------------
// Sets the block type of the block at the given index to the one provided
//
inline void Chunk::SetBlockTypeAtBlockIndex(int blockIndex, const BlockType* blockType)
{
	Block& block = m_blocks[blockIndex];
	block.SetType(blockType);

	// Need to check if this block is now sky
	if (!blockType->m_isFullyOpaque)
	{
		int indexAboveMe = blockIndex + Chunk::BLOCKS_PER_Z_LAYER;
		if (indexAboveMe < Chunk::BLOCKS_PER_CHUNK)
		{
			Block& aboveBlock = m_blocks[indexAboveMe];
			if (aboveBlock.IsPartOfSky())
			{
				block.SetIsPartOfSky(true);
			}
		}
	}


	// Dirty the mesh and adjacent neighbors if the block was on the XY-border of the chunk
	m_isMeshDirty = true;

	IntVector3 blockCoords = GetBlockCoordsFromBlockIndex(blockIndex);

	if (blockCoords.x == 0 && m_westNeighborChunk != nullptr) // West Neighbor
	{
		m_westNeighborChunk->SetIsMeshDirty(true);
	}
	else if (blockCoords.x == CHUNK_DIMENSIONS_X - 1 && m_eastNeighborChunk != nullptr) // East Neighbor
	{
		m_eastNeighborChunk->SetIsMeshDirty(true);
	}

	if (blockCoords.y == 0 && m_southNeighborChunk != nullptr) // South Neighbor
	{
		m_southNeighborChunk->SetIsMeshDirty(true);
	}
	else if (blockCoords.y == CHUNK_DIMENSIONS_Y - 1 && m_northNeighborChunk != nullptr) // North Neighbor
	{
		m_northNeighborChunk->SetIsMeshDirty(true);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the type of the block at the given block coords to the given type
//
inline void Chunk::SetBlockTypeAtBlockCoords(const IntVector3& blockCoords, const BlockType* blockType)
{
	int blockIndex = GetBlockIndexFromBlockCoords(blockCoords);
	SetBlockTypeAtBlockIndex(blockIndex, blockType);
}


//-----------------------------------------------------------------------------------------------
// Returns the block index of the block given by blockCoords
//
inline int Chunk::GetBlockIndexFromBlockCoords(const IntVector3& blockCoords)
{
	int index = BLOCKS_PER_Z_LAYER * blockCoords.z + CHUNK_DIMENSIONS_X * blockCoords.y + blockCoords.x;
	return index;
}


//-----------------------------------------------------------------------------------------------
// Returns the block coords for the block given by blockIndex
//
inline IntVector3 Chunk::GetBlockCoordsFromBlockIndex(int blockIndex)
{
	int xCoord = blockIndex & CHUNK_X_MASK;
	int yCoord = (blockIndex & CHUNK_Y_MASK) >> CHUNK_BITS_Y;
	int zCoord = (blockIndex & CHUNK_Z_MASK) >> CHUNK_BITS_XY;

	return IntVector3(xCoord, yCoord, zCoord);
}
