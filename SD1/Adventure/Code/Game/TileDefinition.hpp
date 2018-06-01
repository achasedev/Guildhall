/************************************************************************/
/* File: TileDefinition.hpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Class to represent a type of tile, defining its characteristics
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

typedef tinyxml2::XMLElement XMLElement;
class SpriteSheet;


class TileDefinition
{
public: 
	//-----Public Methods-----

	explicit TileDefinition(const XMLElement& tileDefElement);

	// Accessors
	std::string		GetName() const;
	AABB2			GetBaseSpriteUVs() const;
	Rgba			GetBaseTint() const;
	AABB2			GetOverlaySpriteUVs() const;
	Rgba			GetOverlayTint() const;
	Rgba			GetChromaKey() const;
	bool			AllowsSight() const;
	bool			AllowsWalking() const;
	bool			AllowsFlying() const;
	bool			AllowsSwimming() const;

	static void LoadDefinitions();
	static const TileDefinition* ParseXMLAttribute(const XMLElement& mapDefinitionElement, const char* attributeName, const TileDefinition* defaultValue);

	static const Texture& GetTileSheetTexture();
	static const TileDefinition* GetTileDefinitionFromColor(const Rgba& color);

	
private:
	//-----Private Methods-----

	void ParseSpriteUVs(const XMLElement& tileDefElement);


private:
	//-----Private Data-----

	std::string m_name				= "";								// Name of the definition
	AABB2 m_baseSpriteUVs			= AABB2::UNIT_SQUARE_OFFCENTER;		// Spritesheet UVs of this tile's base sprite texture
	AABB2 m_overlaySpriteUVs		= AABB2::UNIT_SQUARE_OFFCENTER;		// Spritesheet UVs of this tile's overlay sprite texture
	Rgba m_baseSpriteTint			= Rgba::WHITE;						// Color tint to apply to this sprite's base texture
	Rgba m_overlaySpriteTint		= Rgba::WHITE;						// Color tint to apply to this sprite's overlay texture
	Rgba m_chromaKey				= Rgba::WHITE;						// Color representative of this TileDefinition in a Texel image
	bool m_allowsSight				= true;								// Can entities see through this tile? (Raycasting)
	bool m_allowsWalking			= true;								// Can entities walk on this tile?
	bool m_allowsFlying				= true;								// Can entities fly over this tile?
	bool m_allowsSwimming			= false;							// Can entities swim on this tile?

	// Static list of definitions loaded from file
	static std::map<std::string, TileDefinition*> s_definitions;

	// Static spritesheet that all tile definition sprite coordinates pertain to
	static SpriteSheet* s_tileSpriteSheet;
};