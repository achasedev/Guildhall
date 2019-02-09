/************************************************************************/
/* File: BlockType.cpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Implementation of the BlockType class
/************************************************************************/
#include "Game/Environment/BlockType.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Resources/SpriteSheet.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

// Storage for types and spritesheet
std::map<std::string, uint8_t> BlockType::s_typeNames;
const BlockType* BlockType::s_types[MAX_BLOCK_TYPES];
const SpriteSheet* BlockType::s_spriteSheet = nullptr;


//-----------------------------------------------------------------------------------------------
// Loads the spritesheet for the blocks and creates the block types for use
//
void BlockType::InitializeTypes()
{
	// Set up the sprite sheet and material
	Texture* spriteTexture = AssetDB::CreateOrGetTexture("Data/Images/Terrain_32x32.png");
	s_spriteSheet = new SpriteSheet(*spriteTexture, IntVector2(32, 32));
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Block.xml");

	// Air
	BlockType* airType = new BlockType();
	airType->m_name = "Air";
	airType->m_typeIndex = 0;
	airType->m_topUVs = AABB2(Vector2::ZERO, Vector2::ZERO);
	airType->m_sideUVs = AABB2(Vector2::ZERO, Vector2::ZERO);
	airType->m_bottomUVs = AABB2(Vector2::ZERO, Vector2::ZERO);

	// Grass
	BlockType* grassType = new BlockType();
	grassType->m_name = "Grass";
	grassType->m_typeIndex = 1;
	grassType->m_topUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(21);
	grassType->m_sideUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(99);
	grassType->m_bottomUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(100);

	// Dirt
	BlockType* dirtType = new BlockType();
	dirtType->m_name = "Dirt";
	dirtType->m_typeIndex = 2;
	dirtType->m_topUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(100);
	dirtType->m_sideUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(100);
	dirtType->m_bottomUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(100);

	// Stone
	BlockType* stoneType = new BlockType();
	stoneType->m_name = "Stone";
	stoneType->m_typeIndex = 3;
	stoneType->m_topUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(84);
	stoneType->m_sideUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(84);
	stoneType->m_bottomUVs = s_spriteSheet->GetTexUVsFromSpriteIndex(84);

	// Add them
	AddBlockType(airType);
	AddBlockType(grassType);
	AddBlockType(dirtType);
	AddBlockType(stoneType);

	// Null out all types not filled in
	for (int blockTypeIndex = 4; blockTypeIndex < MAX_BLOCK_TYPES; ++blockTypeIndex)
	{
		s_types[blockTypeIndex] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the block type specified by the index; Returns nullptr if it doesn't exist
//
const BlockType* BlockType::GetTypeByIndex(uint8_t index)
{
	ASSERT_OR_DIE(index > 0 && index < MAX_BLOCK_TYPES, Stringf("Received out of bounds index for BlockType: %i", index).c_str());

	return s_types[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the block type given by the name; Errors and dies if it doesn't exist
//
const BlockType* BlockType::GetTypeByName(const std::string& name)
{
	bool exists = s_typeNames.find(name) != s_typeNames.end();
	ASSERT_OR_DIE(exists, Stringf("BlockType given by name \"%s\" doesn't exist", name.c_str()).c_str());

	uint8_t index = s_typeNames.at(name);
	return GetTypeByIndex(index);
}


//-----------------------------------------------------------------------------------------------
// Adds the given type to the list of types, checking for duplicates
//
void BlockType::AddBlockType(const BlockType* type)
{
	bool nameAlreadyExists = s_typeNames.find(type->m_name) != s_typeNames.end();
	bool typeAlreadyExists = s_types[type->m_typeIndex] != nullptr;

	GUARANTEE_OR_DIE(!nameAlreadyExists, Stringf("Duplicate BlockType name added: \"%s\"", type->m_name.c_str()));
	GUARANTEE_OR_DIE(!typeAlreadyExists, Stringf("Duplicate BlockType index added: %i", type->m_typeIndex).c_str());

	// Safety checks
	GUARANTEE_OR_DIE(type->m_typeIndex >= 0 && type->m_typeIndex < MAX_BLOCK_TYPES, 
		Stringf("Index for BlockType \"%s\" is out of bounds: %i", type->m_name.c_str(), type->m_typeIndex).c_str());
	GUARANTEE_OR_DIE(!IsStringNullOrEmpty(type->m_name), "Attempted to add BlockType with no name");

	s_typeNames[type->m_name] = type->m_typeIndex;
	s_types[type->m_typeIndex] = type;
}
