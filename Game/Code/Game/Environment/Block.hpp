/************************************************************************/
/* File: Block.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Flyweight class to represent a single block in a Chunk
/************************************************************************/
#pragma once
#include <stdint.h>
#include "Game/Environment/BlockType.hpp"

class Block
{
public:
	//-----Public Methods-----

	Block() {}
	Block(uint8_t type, uint8_t light, uint8_t flags);

	inline uint8_t	GetType() const;
	inline void		SetType(uint8_t blockType);

	// Accessors for the type
	inline bool IsFullyOpaque() const;
	inline bool IsSolid() const;


public:
	//-----Public Data-----

	static Block MISSING_BLOCK; // For references outside chunk space, etc.
	

private:
	//-----Private Data-----

	uint8_t m_type = 0; // Default blocks to air type
	uint8_t m_light = 0;
	uint8_t m_flags = 0;

};


//-----------------------------------------------------------------------------------------------
// Returns the type of the block
//
inline uint8_t Block::GetType() const
{
	return m_type;
}


//-----------------------------------------------------------------------------------------------
// Sets the block's type to be the one given
//
inline void Block::SetType(uint8_t blockType)
{
	m_type = blockType;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block fully blocks light and vision
//
inline bool Block::IsFullyOpaque() const
{
	const BlockType* blockType = BlockType::GetTypeByIndex(m_type);
	return blockType->m_isFullyOpaque;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block will block raycasts and has collsion
//
inline bool Block::IsSolid() const
{
	const BlockType* blockType = BlockType::GetTypeByIndex(m_type);
	return blockType->m_isSolid;
}