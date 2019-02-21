/************************************************************************/
/* File: BlockType.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Class to represent data/constant attributes for a block kind
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include <map>
#include <string>

#define MAX_BLOCK_TYPES (256)

class SpriteSheet;

class BlockType
{
public:
	//-----Public Methods-----

	static void InitializeTypes();

	static const BlockType* GetTypeByIndex(uint8_t index);
	static const BlockType* GetTypeByName(const std::string& name);


private:
	//-----Private Methods-----

	static void AddBlockType(const BlockType& type);


public:
	//-----Public Member Data-----

	std::string m_name = "NOT USED";
	uint8_t		m_typeIndex = AIR_TYPE_INDEX; // Default to air
	AABB2		m_topUVs = AABB2::UNIT_SQUARE_OFFCENTER;
	AABB2		m_sideUVs = AABB2::UNIT_SQUARE_OFFCENTER;
	AABB2		m_bottomUVs = AABB2::UNIT_SQUARE_OFFCENTER;

	bool		m_isFullyOpaque = false;			// Does it fully obscure other blocks behind it?
	bool		m_isSolid = false;					// Does it block raycasts and have collision?

	static constexpr uint8_t AIR_TYPE_INDEX = 0;
	static constexpr uint8_t MISSING_TYPE_INDEX = 1;


private:
	//-----Private Static Data-----

	// Organizing Types
	static std::map<std::string, uint8_t> s_typeNames;
	static BlockType s_types[MAX_BLOCK_TYPES];

	// Rendering
	static const SpriteSheet* s_spriteSheet;

};
