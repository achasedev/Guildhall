/************************************************************************/
/* File: Block.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Flyweight class to represent a single block in a Chunk
/************************************************************************/
#pragma once
#include <stdint.h>

class Block
{
public:
	//-----Public Methods-----

	Block() {}

	uint8_t GetType() const;
	void	SetType(uint8_t blockType);


private:
	//-----Private Data-----

	uint8_t m_type = 0; // Default blocks to air type
	uint8_t m_light = 0;
	uint8_t m_flags = 0;

};
