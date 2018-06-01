/************************************************************************/
/* File: Block.hpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Class to represent a single block of the map
/************************************************************************/
#pragma once

class BlockDefinition;

class Block
{
public:
	//-----Public Methods-----

	Block();
	~Block();

	void				SetType(BlockDefinition* newType);
	BlockDefinition*	GetType() const;
	inline bool			isAir() const { return m_definition != nullptr; }


private:
	//-----Private Data-----

	BlockDefinition* m_definition;
};