/************************************************************************/
/* File: BlockDefinition.cpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Implementation of the BlockDefinition class
/************************************************************************/
#include "Game/BlockDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

std::map<std::string, BlockDefinition*> BlockDefinition::s_definitions;	// registry of definitions
SpriteSheet* BlockDefinition::s_blockTextureAtlas = nullptr;				// Texture used by all blocks


//-----------------------------------------------------------------------------------------------
// Constructor
//
BlockDefinition::BlockDefinition(Texture* texture, const AABB2& topUVs, const AABB2& sideUVs, const AABB2& bottomUVs)
	: m_texture(texture)
	, m_topUVs(topUVs)
	, m_sideUVs(sideUVs)
	, m_bottomUVs(bottomUVs)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BlockDefinition::~BlockDefinition()
{
}


//-----------------------------------------------------------------------------------------------
// Constructs all Block definitions and adds them to the registry
//
void BlockDefinition::LoadDefinitions()
{
	// Create the sprite atlas
	Renderer* renderer	= Renderer::GetInstance();
	Texture* spriteAtlasTexture = renderer->CreateOrGetTexture("Data/Images/BlockTextureAtlas.png");
	s_blockTextureAtlas = new SpriteSheet("BlockAtlas", *spriteAtlasTexture, IntVector2(32, 32));

	// Grass block
	AABB2 topUVs = s_blockTextureAtlas->GetTexUVsFromSpriteIndex(21);
	AABB2 sideUVs = s_blockTextureAtlas->GetTexUVsFromSpriteIndex(99);
	AABB2 bottomUVs = s_blockTextureAtlas->GetTexUVsFromSpriteIndex(100);
	s_definitions["Grass"] = new BlockDefinition(spriteAtlasTexture, topUVs, sideUVs, bottomUVs);
}


//-----------------------------------------------------------------------------------------------
// Returns the block definition give by name
//
BlockDefinition* BlockDefinition::GetBlockDefinitionByName(const std::string& name)
{
	bool definitionExists = s_definitions.find(name) != s_definitions.end();
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: BlockDefinition \"%s\" doesn't exist!", name.c_str()));

	return s_definitions[name];
}


//-----------------------------------------------------------------------------------------------
// Returns a reference to the sprite sheet used by all blocks
//
SpriteSheet* BlockDefinition::GetBlockTextureAtlas()
{
	return s_blockTextureAtlas;
}


//-----------------------------------------------------------------------------------------------
// Returns the UVs of the top face of the block
//
AABB2 BlockDefinition::GetTopUVs() const
{
	return m_topUVs;
}


//-----------------------------------------------------------------------------------------------
// Returns the UVs of the side face of the block (all side faces have the same texture)
//
AABB2 BlockDefinition::GetSideUVs() const
{
	return m_sideUVs;
}


//-----------------------------------------------------------------------------------------------
// Returns the UVs of the bottom face of the block
//
AABB2 BlockDefinition::GetBottomUVs() const
{
	return m_bottomUVs;
}
