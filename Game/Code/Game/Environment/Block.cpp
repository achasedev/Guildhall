#include "Game/Environment/Block.hpp"


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

