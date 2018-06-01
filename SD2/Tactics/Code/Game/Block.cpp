/************************************************************************/
/* File: Block.cpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Implementation of the Block class
/************************************************************************/
#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
Block::Block()
	: m_definition(nullptr)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Block::~Block()
{

}


//-----------------------------------------------------------------------------------------------
// Sets this block's type to the one given
//
void Block::SetType(BlockDefinition* newType)
{
	m_definition = newType;
}


//-----------------------------------------------------------------------------------------------
// Returns this Block's BlockDefinition
//
BlockDefinition* Block::GetType() const
{
	return m_definition;
}
