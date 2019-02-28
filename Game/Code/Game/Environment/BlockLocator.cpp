/************************************************************************/
/* File: BlockLocator.cpp
/* Author: Andrew Chase
/* Date: February 18th 2019
/* Description: Implementation of the BlockLocator class
/************************************************************************/
#include "Game/Environment/Chunk.inl"
#include "Game/Environment/Block.hpp"
#include "Game/Environment/BlockLocator.inl"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector3.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
BlockLocator::BlockLocator(Chunk* chunk, int blockIndex)
	: m_chunk(chunk), m_blockIndex(blockIndex)
{
}


//-----------------------------------------------------------------------------------------------
// Operator - returns true if both locators point to the exact same block
//
bool BlockLocator::operator==(const BlockLocator& compare) const
{
	return (m_chunk == compare.m_chunk && m_blockIndex == compare.m_blockIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns a locator to the block that is east of this block
//
BlockLocator BlockLocator::ToEast() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the east boundary of this chunk, move into the next chunk
	if ((m_blockIndex & Chunk::CHUNK_X_MASK) == Chunk::CHUNK_X_MASK)
	{
		int newBlockIndex = m_blockIndex & ~Chunk::CHUNK_X_MASK;
		return BlockLocator(m_chunk->GetEastNeighbor(), newBlockIndex);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex + 1);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a locator to the block that is west of this block
//
BlockLocator BlockLocator::ToWest() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the west boundary of this chunk, move into the next chunk
	if ((m_blockIndex & Chunk::CHUNK_X_MASK) == 0)
	{
		int newBlockIndex = m_blockIndex | Chunk::CHUNK_X_MASK;
		return BlockLocator(m_chunk->GetWestNeighbor(), newBlockIndex);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex - 1);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a locator to the block that is north of this block
//
BlockLocator BlockLocator::ToNorth() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the north boundary of this chunk, move into the next chunk
	if ((m_blockIndex & Chunk::CHUNK_Y_MASK) == Chunk::CHUNK_Y_MASK)
	{
		int newBlockIndex = m_blockIndex & ~Chunk::CHUNK_Y_MASK;
		return BlockLocator(m_chunk->GetNorthNeighbor(), newBlockIndex);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex + Chunk::CHUNK_DIMENSIONS_X);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a locator to the block that is south of this block
//
BlockLocator BlockLocator::ToSouth() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the south boundary of this chunk, move into the next chunk
	if ((m_blockIndex & Chunk::CHUNK_Y_MASK) == 0)
	{
		int newBlockIndex = m_blockIndex | Chunk::CHUNK_Y_MASK;
		return BlockLocator(m_chunk->GetSouthNeighbor(), newBlockIndex);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex - Chunk::CHUNK_DIMENSIONS_X);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a locator to the block that is above this block
//
BlockLocator BlockLocator::ToAbove() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the top of the chunk return a missing block locator
	if ((m_blockIndex & Chunk::CHUNK_Z_MASK) == Chunk::CHUNK_Z_MASK)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex + Chunk::BLOCKS_PER_Z_LAYER);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a locater to the block that is below this block
//
BlockLocator BlockLocator::ToBelow() const
{
	// Don't do anything on an invalid block locator
	if (m_chunk == nullptr)
	{
		return BlockLocator(nullptr, 0);
	}

	// If we're at the bottom of the chunk return a missing block locator
	if ((m_blockIndex & Chunk::CHUNK_Z_MASK) == 0)
	{
		return BlockLocator(nullptr, -1);
	}
	else
	{
		return BlockLocator(m_chunk, m_blockIndex - Chunk::BLOCKS_PER_Z_LAYER);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the block that is at the block coordinates (this block coords + blockCoordOffset)
//
BlockLocator BlockLocator::StepInCoordDirection(const IntVector3& blockCoordOffset) const
{
	BlockLocator finalResult = *this;

	bool towardsEast = blockCoordOffset.x > 0;
	bool towardsNorth = blockCoordOffset.y > 0;
	bool towardsUp = blockCoordOffset.z > 0;

	IntVector3 numSteps = IntVector3(AbsoluteValue(blockCoordOffset.x), AbsoluteValue(blockCoordOffset.y), AbsoluteValue(blockCoordOffset.z));

	for (int xStep = 0; xStep < numSteps.x; ++xStep)
	{
		if (towardsEast)
		{
			finalResult = finalResult.ToEast();
		}
		else
		{
			finalResult = finalResult.ToWest();
		}
	}

	for (int yStep = 0; yStep < numSteps.y; ++yStep)
	{
		if (towardsNorth)
		{
			finalResult = finalResult.ToNorth();
		}
		else
		{
			finalResult = finalResult.ToSouth();
		}
	}

	for (int zStep = 0; zStep < numSteps.z; ++zStep)
	{
		if (towardsUp)
		{
			finalResult = finalResult.ToAbove();
		}
		else
		{
			finalResult = finalResult.ToBelow();
		}
	}

	return finalResult;
}


//-----------------------------------------------------------------------------------------------
// Returns this block's center position in world coordinates
//
Vector3 BlockLocator::GetBlockCenterWorldPosition() const
{
	if (m_chunk == nullptr)
	{
		return Vector3::ZERO;
	}

	Vector3 chunkOrigin = m_chunk->GetOriginWorldPosition();
	IntVector3 blockCoords = m_chunk->GetBlockCoordsFromBlockIndex(m_blockIndex);
	Vector3 blockCenterPosition = chunkOrigin + blockCoords.GetAsFloats() + Vector3(0.5f);

	return blockCenterPosition;
}
