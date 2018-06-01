/************************************************************************/
/* File: IsoSprite.cpp
/* Author: Andrew Chase
/* Date: February 26th, 2017
/* Description: Implementation of the IsoSprite class
/************************************************************************/
#include "Game/IsoSprite.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

// Map of all IsoSprites for the game
std::map<std::string, IsoSprite*> IsoSprite::s_isoSprites;


//-----------------------------------------------------------------------------------------------
// Loads the XML document given by filePath and populates the IsoSprite registry
//
void IsoSprite::LoadIsoSprites(const std::string& filePath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: IsoSprite::LoadIsoSprites() couldn't load file \"%s\"", filePath.c_str()));

	XMLElement* rootElement = document.RootElement();

	// Get the spritesheet
	std::string spriteSheetName = ParseXmlAttribute(*rootElement, "spritesheet");
	SpriteSheet* spriteSheet	= SpriteSheet::GetResource(spriteSheetName);

	XMLElement* isoSpriteElement = rootElement->FirstChildElement();

	while (isoSpriteElement != nullptr)
	{
		// Pull out the name
		std::string isoSpriteName = ParseXmlAttribute(*isoSpriteElement, "name");

		// Construct the IsoSprite
		IsoSprite* isoSprite = new IsoSprite(isoSpriteName, *isoSpriteElement, *spriteSheet);

		// Add to registry
		AddIsoSprite(isoSprite);

		// Move on to next element
		isoSpriteElement = isoSpriteElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the sprite to the registry, throwing an error if a sprite with the given name already exists
//
void IsoSprite::AddIsoSprite(IsoSprite* spriteToAdd)
{
	std::string spriteName = spriteToAdd->GetName();
	bool alreadyExists= (s_isoSprites.find(spriteName) != s_isoSprites.end());
	GUARANTEE_RECOVERABLE(!alreadyExists, Stringf("Warning: IsoSprite::AddDefinition tried to add duplicate IsoSprite \"%s\"", spriteName.c_str()));

	s_isoSprites[spriteName] = spriteToAdd;
}


//-----------------------------------------------------------------------------------------------
// Returns the IsoSprite in the registry identified by spriteName
//
IsoSprite* IsoSprite::GetIsoSprite(const std::string& spriteName)
{
	if (s_isoSprites.find(spriteName) != s_isoSprites.end())
	{
		return s_isoSprites.at(spriteName);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Converts an IntVector2 representation of a facing direction into the corresponding enumeration
//
IsoDirection IsoSprite::ConvertStringToDirection(const std::string& text)
{
	IsoDirection direction = ISO_DIRECTION_INVALID;

	if		(text == "away_left")		{ direction = ISO_DIRECTION_AWAY_LEFT; }
	else if (text == "away_right")		{ direction = ISO_DIRECTION_AWAY_RIGHT; }
	else if (text == "towards_left")	{ direction = ISO_DIRECTION_TOWARDS_LEFT; }
	else if (text == "towards_right")	{ direction = ISO_DIRECTION_TOWARDS_RIGHT; }

	return direction;
}


//-----------------------------------------------------------------------------------------------
// Constructor - from 4 sprites
//
IsoSprite::IsoSprite(const std::string& name, const Sprite* towardsLeft, const Sprite* towardsRight, const Sprite* awayLeft, const Sprite* awayRight)
{
	m_name = name;
	m_sprites[ISO_DIRECTION_TOWARDS_LEFT]	= towardsLeft;
	m_sprites[ISO_DIRECTION_TOWARDS_RIGHT]	= towardsRight;
	m_sprites[ISO_DIRECTION_AWAY_LEFT]		= awayLeft;
	m_sprites[ISO_DIRECTION_AWAY_RIGHT]		= awayRight;
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite given by the given isometric direction
//
IsoSprite::IsoSprite(const std::string& name, const XMLElement& element, const SpriteSheet& spriteSheet)
{
	m_name = name;

	// Assumes that there is a sprite for each direction - does not create new ones here
	const XMLElement* subElement = element.FirstChildElement();

	while (subElement != nullptr)
	{
		// Get the sprite from the spritesheet
		std::string spriteName = ParseXmlAttribute(*subElement, "sprite");
		Sprite* sprite = spriteSheet.GetSprite(spriteName);
		GUARANTEE_OR_DIE(sprite != nullptr, Stringf("Error: IsoSprite constructor from XML couldn't find sprite \"%s\"", spriteName.c_str()));

		// Get the direction
		std::string directionText	= ParseXmlAttribute(*subElement, "direction");
		IsoDirection direction		= ConvertStringToDirection(directionText);

		// Add the sprite to the collection
		m_sprites[direction] = sprite;

		subElement = subElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the name of this IsoSprite
//
std::string IsoSprite::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite given by the given isometric direction
//
const Sprite* IsoSprite::GetSpriteForDirection(IsoDirection direction) const
{
	return m_sprites[direction];
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite that should be drawn given the actor and camera's forward directions
//
const Sprite* IsoSprite::GetSpriteForViewAngle(const Vector3& actorForward, const Vector3& cameraForward) const
{
	Vector2 actorXZ = actorForward.xz();
	Vector2 camXZForward = cameraForward.xz();
	Vector2 camXZRight = Vector2(camXZForward.y, -camXZForward.x);

	bool isFacingAway	= DotProduct(camXZForward, actorXZ) >= 0.0f;
	bool isFacingRight	= DotProduct(camXZRight, actorXZ) >= 0.0f;

	if (isFacingAway)
	{
		return (isFacingRight ? m_sprites[ISO_DIRECTION_AWAY_RIGHT] : m_sprites[ISO_DIRECTION_AWAY_LEFT]);
	}
	else
	{
		return (isFacingRight ? m_sprites[ISO_DIRECTION_TOWARDS_RIGHT] : m_sprites[ISO_DIRECTION_TOWARDS_LEFT]);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the sprite at the given direction to the one given
//
void IsoSprite::SetSprite(IsoDirection direction, const Sprite* sprite)
{
	m_sprites[direction] = sprite;
}
