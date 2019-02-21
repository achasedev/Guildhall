/************************************************************************/
/* File: BlockLocator.hpp
/* Author: Andrew Chase
/* Date: February 18th 2019
/* Description: Class to represent a block iterator
/************************************************************************/
#pragma once
#include "Engine/Math/Vector3.hpp"

class Chunk;
class Block;

class BlockLocator
{
public:
	//-----Public Methods-----

	BlockLocator(Chunk* chunk, int blockIndex);

	bool operator==(const BlockLocator& compare) const;

	// Accessors
	Block&	GetBlock();
	Chunk*	GetChunk();
	int		GetBlockIndex() const;

	bool	IsValid() const;

	// Producers
	BlockLocator	ToEast() const;
	BlockLocator	ToWest() const;
	BlockLocator	ToNorth() const;
	BlockLocator	ToSouth() const;
	BlockLocator	ToAbove() const;
	BlockLocator	ToBelow() const;
	BlockLocator	StepInCoordDirection(const IntVector3& blockCoordOffset) const;

	Vector3			GetBlockCenterWorldPosition() const;


private:
	//-----Private Data-----

	Chunk*	m_chunk = nullptr;
	int		m_blockIndex = -1;

};
