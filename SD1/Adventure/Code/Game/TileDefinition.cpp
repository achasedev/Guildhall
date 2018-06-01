/************************************************************************/
/* File: TileDefinition.cpp
/* Author: Andrew Chase
/* Date: November 1st, 2017
/* Bugs: None
/* Description: Implementation of the TileDefinition class
/************************************************************************/
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

typedef tinyxml2::XMLDocument XMLDocument;

// Static list of definitions created from file
std::map<std::string, TileDefinition*> TileDefinition::s_definitions;

// Spritesheet that all tile definitions use
SpriteSheet* TileDefinition::s_tileSpriteSheet;


//-----------------------------------------------------------------------------------------------
// Loads the TileDefinition XML file from disk and constructs all the TileDefinitions from its contents
//
TileDefinition::TileDefinition(const XMLElement& tileDefElement)
{
	m_name				= ParseXmlAttribute(tileDefElement, "name", m_name);
	m_baseSpriteTint	= ParseXmlAttribute(tileDefElement, "baseSpriteTint", m_baseSpriteTint);
	m_overlaySpriteTint	= ParseXmlAttribute(tileDefElement, "overlaySpriteTint", m_overlaySpriteTint);
	m_chromaKey			= ParseXmlAttribute(tileDefElement, "chromaKey", m_chromaKey);
	m_allowsSight		= ParseXmlAttribute(tileDefElement, "allowsSight", m_allowsSight);
	m_allowsWalking		= ParseXmlAttribute(tileDefElement, "allowsWalking", m_allowsWalking);
	m_allowsFlying		= ParseXmlAttribute(tileDefElement, "allowsFlying", m_allowsFlying);
	m_allowsSwimming	= ParseXmlAttribute(tileDefElement, "allowsSwimming", m_allowsSwimming);

	// Store the actual UVs instead of the sprite coords
	ParseSpriteUVs(tileDefElement);
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this definition
//
std::string TileDefinition::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the base UV coordinates of this definition's texture segment in the tile spritesheet
//
AABB2 TileDefinition::GetBaseSpriteUVs() const
{
	return m_baseSpriteUVs;
}


//-----------------------------------------------------------------------------------------------
// Returns the base color tint of this definition
//
Rgba TileDefinition::GetBaseTint() const
{
	return m_baseSpriteTint;
}


//-----------------------------------------------------------------------------------------------
// Returns the overlay UV coordinates of this definition's texture segment in the tile spritesheet
//
AABB2 TileDefinition::GetOverlaySpriteUVs() const
{
	return m_overlaySpriteUVs;
}


//-----------------------------------------------------------------------------------------------
// Returns the overlay color tint of this definition
//
Rgba TileDefinition::GetOverlayTint() const
{
	return m_overlaySpriteTint;
}


//-----------------------------------------------------------------------------------------------
// Returns the color that represents this tile definition in a texel image file
//
Rgba TileDefinition::GetChromaKey() const
{
	return m_chromaKey;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this definition allows entities to see through it
//
bool TileDefinition::AllowsSight() const
{
	return m_allowsSight;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this definition allows entities to walk through it
//
bool TileDefinition::AllowsWalking() const
{
	return m_allowsWalking;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this definition allows entities to fly over/through it
//
bool TileDefinition::AllowsFlying() const
{
	return m_allowsFlying;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this definition allows entities to swim through it
//
bool TileDefinition::AllowsSwimming() const
{
	return m_allowsSwimming;
}


//-----------------------------------------------------------------------------------------------
// Static method that loads the TileDefinition XML file and constructs the definitions from its contents
//
void TileDefinition::LoadDefinitions()
{
	// Load the XML file
	XMLDocument tileDefinitionDocument;
	tileDefinitionDocument.LoadFile("Data/Definitions/TileDefinition.xml");

	// Initialize the tile spritesheet
	XMLElement* rootElement					= tileDefinitionDocument.RootElement();
	std::string tileSpriteSheetFileName		= ParseXmlAttribute(*rootElement, "spriteSheet", nullptr);
	IntVector2	spriteLayout				= ParseXmlAttribute(*rootElement, "spriteLayout", spriteLayout);
	Texture*	tileTexture					= g_theRenderer->CreateOrGetTexture(Stringf("Data/Images/%s", tileSpriteSheetFileName.c_str()));
				s_tileSpriteSheet			= new SpriteSheet(*tileTexture, spriteLayout);


	// Iterate across the ActorDefinition elements and create them
	XMLElement* currDefinitionElement = tileDefinitionDocument.RootElement()->FirstChildElement();

	while (currDefinitionElement != nullptr)
	{
		TileDefinition* newDefinition = new TileDefinition(*currDefinitionElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_definitions.find(newDefinition->GetName()) != s_definitions.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate Tile definition in TileDefinition.xml - \"%s\"", newDefinition->GetName().c_str()));
		
		// Add the definition to the map
		s_definitions[newDefinition->GetName()] = newDefinition;

		// Move to the next element
		currDefinitionElement = currDefinitionElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the given XML element for the given attribute and interprets it as the name of a TileDefinition,
// returning the corresponding TileDefinition* from the static array
//
const TileDefinition* TileDefinition::ParseXMLAttribute(const tinyxml2::XMLElement& mapDefinitionElement, const char* attributeName, const TileDefinition* defaultValue)
{
	std::string defaultTileName = ParseXmlAttribute(mapDefinitionElement, attributeName, nullptr);

	if (defaultTileName.compare(std::string("")) == 0)
	{
		return defaultValue;
	}

	// Safety check - ensure the definition exists
	bool definitionExists = (s_definitions.find(defaultTileName) != s_definitions.end());
	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: Map definition's default tile doesn't exist - tile name was \"%s\"", defaultTileName.c_str()));

	return s_definitions[defaultTileName];
}


//-----------------------------------------------------------------------------------------------
// Returns the texture associated with the TileDefinition static tile sprite sheet
//
const Texture& TileDefinition::GetTileSheetTexture()
{
	return s_tileSpriteSheet->GetTexture();
}


//-----------------------------------------------------------------------------------------------
// Returns the tile definition associated with the given color
//
const TileDefinition* TileDefinition::GetTileDefinitionFromColor(const Rgba& searchColor)
{
	TileDefinition* colorDefinition = nullptr;

	std::map<std::string, TileDefinition*>::iterator itr = s_definitions.begin();

	for (itr; itr != s_definitions.end(); itr++)
	{
		Rgba defColor = itr->second->GetChromaKey();

		// Check color values only, not alpha
		if (searchColor.r == defColor.r && searchColor.g == defColor.g && searchColor.b == defColor.b)
		{
			colorDefinition = itr->second;
			break;
		}
	}

	// Ensure we found a TileDefinition with the given color
	GUARANTEE_OR_DIE(colorDefinition != nullptr, Stringf("Error: TileDefinition::GetTileDefinitionFromColor couldn't find definition with FromFileColor \"%d,%d,%d\"", searchColor.r, searchColor.g, searchColor.b));

	return colorDefinition;
}


//-----------------------------------------------------------------------------------------------
// Parses the tile definition element for the base and overlay spritesheet coordinates and stores
// the corresponding UVs in the definition
//
void TileDefinition::ParseSpriteUVs(const XMLElement& tileDefElement)
{
	// Base
	IntVector2 baseSpriteCoords = ParseXmlAttribute(tileDefElement, "baseSpriteCoords", IntVector2(-1, -1));

	if (baseSpriteCoords != IntVector2(-1, -1))
	{
		m_baseSpriteUVs = s_tileSpriteSheet->GetTexUVsFromSpriteCoords(baseSpriteCoords);
	}

	// Overlay
	IntVector2 overlaySpriteCoords = ParseXmlAttribute(tileDefElement, "overlaySpriteCoords", IntVector2(-1, -1));

	if (overlaySpriteCoords != IntVector2(-1, -1))
	{
		m_overlaySpriteUVs = s_tileSpriteSheet->GetTexUVsFromSpriteCoords(overlaySpriteCoords);
	}
}
