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
	: m_type(type), m_light(light), m_flags(flags)
{
}
