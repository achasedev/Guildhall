/************************************************************************/
/* File: BlockDefinition.hpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Class to represent single type of block, and to hold
				the block type registry
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <map>

class Texture;
class SpriteSheet;

class BlockDefinition
{
public:
	//-----Public Methods-----

	BlockDefinition(Texture* texture, const AABB2& topUVs, const AABB2& sideUVs, const AABB2& bottomUVs);
	~BlockDefinition();

	static void				LoadDefinitions();
	static BlockDefinition*	GetBlockDefinitionByName(const std::string& name);
	static SpriteSheet*		GetBlockTextureAtlas();

	AABB2 GetTopUVs() const;
	AABB2 GetSideUVs() const;
	AABB2 GetBottomUVs() const;


private:
	//-----Private Data-----

	std::string m_name;

	Texture*	m_texture;
	AABB2		m_topUVs;
	AABB2		m_sideUVs;
	AABB2		m_bottomUVs;

	static SpriteSheet* s_blockTextureAtlas;
	static std::map<std::string, BlockDefinition*> s_definitions;
};
