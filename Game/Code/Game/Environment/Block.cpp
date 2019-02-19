/************************************************************************/
/* File: Block.cpp
/* Author: Andrew Chase
/* Date: February 18th 2019
/* Description: Implementation of the Block class
/************************************************************************/
#include "Game/Environment/Block.hpp"
#include "Game/Environment/BlockType.hpp"

// Missing block
Block Block::MISSING_BLOCK = Block(BlockType::MISSING_TYPE_INDEX, 0, 0);


//-----------------------------------------------------------------------------------------------
// Constructor
//
Block::Block(uint8_t type, uint8_t light, uint8_t flags)
{

}


//-----------------------------------------------------------------------------------------------
// Returns the type of the block
//
uint8_t Block::GetType() const
{
	return m_type;
}


//-----------------------------------------------------------------------------------------------
// Sets the block's type to be the one given
//
void Block::SetType(uint8_t blockType)
{
	m_type = blockType;
}

