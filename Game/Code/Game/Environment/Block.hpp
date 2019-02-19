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
	Block(uint8_t type, uint8_t light, uint8_t flags);

	uint8_t GetType() const;
	void	SetType(uint8_t blockType);


public:
	//-----Public Data-----

	static Block MISSING_BLOCK; // For references outside chunk space, etc.
	

private:
	//-----Private Data-----

	uint8_t m_type = 0; // Default blocks to air type
	uint8_t m_light = 0;
	uint8_t m_flags = 0;

};
