/************************************************************************/
/* File: BlockLocator.inl
/* Author: Andrew Chase
/* Date: February 28th 2019
/* Description: File for inline functions for BlockLocator
/************************************************************************/
#include "Game/Environment/Block.hpp"
#include "Game/Environment/Chunk.hpp"
#include "Game/Environment/BlockLocator.hpp"


//-----------------------------------------------------------------------------------------------
// Returns the block this locator references, or the missing block if it doesn't exist
//
inline Block& BlockLocator::GetBlock()
{
	if (m_chunk == nullptr)
	{
		return Block::MISSING_BLOCK;
	}
	else
	{
		return m_chunk->GetBlock(m_blockIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the chunk that contains the block this locator locates
//
inline Chunk* BlockLocator::GetChunk()
{
	return m_chunk;
}


//-----------------------------------------------------------------------------------------------
// Returns the block index of the block this locator points to
//
inline int BlockLocator::GetBlockIndex() const
{
	return m_blockIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the block coordinates for this block within its chunk
//
inline IntVector3 BlockLocator::GetBlockCoords() const
{
	if (m_chunk == nullptr)
	{
		return IntVector3(0, 0, 0);
	}

	return m_chunk->GetBlockCoordsFromBlockIndex(m_blockIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns whether this block locator is a valid locator
//
inline bool BlockLocator::IsValid() const
{
	return (m_chunk != nullptr);
}
