/************************************************************************/
/* File: IsoSprite.hpp
/* Author: Andrew Chase
/* Date: February 26th, 2017
/* Description: Class to represent a collection of 4 isometric sprites
/************************************************************************/
#pragma once
#include <map>
#include "Engine/Math/Vector3.hpp"
#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Core/XmlUtilities.hpp"

// Enum for the 4 isometric directions
enum IsoDirection
{
	ISO_DIRECTION_INVALID = -1,
	ISO_DIRECTION_TOWARDS_LEFT,
	ISO_DIRECTION_TOWARDS_RIGHT,
	ISO_DIRECTION_AWAY_LEFT,
	ISO_DIRECTION_AWAY_RIGHT,
	NUM_ISO_DIRECTIONS
};


class IsoSprite
{
public:
	//-----Public Methods-----

	// Constructor - given a sprite for each direction
	IsoSprite(const std::string& name, const Sprite* towardsLeft, const Sprite* towardsRight, const Sprite* awayLeft, const Sprite* awayRight);

	// Accessors
	std::string		GetName() const;
	const Sprite*	GetSpriteForDirection(IsoDirection direction) const;
	const Sprite*	GetSpriteForViewAngle(const Vector3& actorForward, const Vector3& cameraForward) const;

	// Mutators
	void SetSprite(IsoDirection direction, const Sprite* sprite);

	// Statics
	static void			LoadIsoSprites(const std::string& filePath);
	static void			AddIsoSprite(IsoSprite* spriteToAdd);
	static IsoSprite*	GetIsoSprite(const std::string& spriteName);


private:
	//-----Private Methods-----

	// Constructor - given an XMLElement, only to be used in LoadDefinitions()
	IsoSprite(const std::string& name, const XMLElement& element, const SpriteSheet& spriteSheet);

	// Converts the string representation of the isometric direction to the appropriate enum
	static IsoDirection ConvertStringToDirection(const std::string& direction);


private:
	//-----Private Data-----

	std::string		m_name;
	const Sprite*	m_sprites[4];	// 4 directions for sprites

	// Map of all IsoSprites for the game
	static std::map<std::string, IsoSprite*> s_isoSprites;

};
