/************************************************************************/
/* File: Block.hpp
/* Author: Andrew Chase
/* Date: February 9th 2019
/* Description: Flyweight class to represent a single block in a Chunk
/************************************************************************/
#pragma once
#include <stdint.h>
#include "Game/Environment/BlockType.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"

class Block
{
public:
	//-----Public Methods-----

	Block() {}
	Block(uint8_t type, uint8_t light, uint8_t flags);


	// Accessors
	inline uint8_t			GetTypeIndex() const;
	inline const BlockType* GetType() const;

	inline bool		IsPartOfSky() const;
	inline bool		IsLightingDirty() const;
	inline bool		IsFullyOpaque() const;
	inline bool		IsVisible() const;
	inline bool		IsSolid() const;

	inline int		GetOutdoorLight() const;
	inline int		GetIndoorLight() const;
	inline Rgba		GetLightingAsRGBChannels() const;

	// Mutators
	inline void		SetType(const BlockType* blockType);
	inline void		SetIsPartOfSky(bool isPartOfSky);
	inline void		SetIsLightingDirty(bool isLightingDirty);
	inline void		SetIsFullyOpaque(bool isFullyOpaque);
	inline void		SetIsVisible(bool isVisible);
	inline void		SetIsSolid(bool isSolid);

	inline void		SetOutdoorLighting(int outdoorLightValue);
	inline void		SetIndoorLighting(int indoorLightValue);


public:
	//-----Public Data-----

	static Block MISSING_BLOCK; // For references outside chunk space, etc.
	
	static constexpr uint8_t BLOCK_MAX_LIGHTING			= 15;
	static constexpr uint8_t BLOCK_OUTDOOR_LIGHT_MASK	= 0b11110000;
	static constexpr uint8_t BLOCK_INDOOR_LIGHT_MASK	= 0b00001111;

	static constexpr uint8_t BLOCK_BIT_IS_SKY			= 0b10000000;
	static constexpr uint8_t BLOCK_BIT_IS_LIGHT_DIRTY	= 0b01000000;
	static constexpr uint8_t BLOCK_BIT_IS_FULLY_OPAQUE	= 0b00100000;
	static constexpr uint8_t BLOCK_BIT_IS_VISIBLE		= 0b00010000;
	static constexpr uint8_t BLOCK_BIT_IS_SOLID			= 0b00001000;


private:
	//-----Private Data-----

	uint8_t m_typeIndex = 0; // Default blocks to air type
	uint8_t m_light = 0;
	uint8_t m_flags = 0;

};


//-----------------------------------------------------------------------------------------------
// Returns the type index of the block
//
inline uint8_t Block::GetTypeIndex() const
{
	return m_typeIndex;
}


//-----------------------------------------------------------------------------------------------
// Returns the type of the block
//
inline const BlockType* Block::GetType() const
{
	return BlockType::GetTypeByIndex(m_typeIndex);
}


//-----------------------------------------------------------------------------------------------
// Sets the block's type to be the one given
//
inline void Block::SetType(const BlockType* blockType)
{
	m_typeIndex = blockType->m_typeIndex;

	// Also set the helper flags!
	SetIsFullyOpaque(blockType->m_isFullyOpaque);
	SetIsSolid(blockType->m_isSolid);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block is part of the sky (i.e. is not opaque AND has no opaque blocks
// directly above it)
//
inline bool Block::IsPartOfSky() const
{
	return (m_flags & BLOCK_BIT_IS_SKY);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block has dirty lighting, meaning it is in the dirty lighting list
//
inline bool Block::IsLightingDirty() const
{
	return (m_flags & BLOCK_BIT_IS_LIGHT_DIRTY);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block fully blocks light and vision
//
inline bool Block::IsFullyOpaque() const
{
	return (m_flags & BLOCK_BIT_IS_FULLY_OPAQUE);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block is visible....not sure what it means yet....
//
inline bool Block::IsVisible() const
{
	return (m_flags & BLOCK_BIT_IS_VISIBLE);
}


//-----------------------------------------------------------------------------------------------
// Returns true if this block will block raycasts and has collsion
//
inline bool Block::IsSolid() const
{
	return (m_flags & BLOCK_BIT_IS_SOLID);
}


//-----------------------------------------------------------------------------------------------
// Returns the outdoor light value this block possesses
//
inline int Block::GetOutdoorLight() const
{
	return (m_light & BLOCK_OUTDOOR_LIGHT_MASK);
}


//-----------------------------------------------------------------------------------------------
// Returns the indoor light value this block possesses
//
inline int Block::GetIndoorLight() const
{
	return (m_light & BLOCK_INDOOR_LIGHT_MASK);
}


//-----------------------------------------------------------------------------------------------
// Returns the indoor and outdoor lighting, for building a mesh with lighting
//
inline Rgba Block::GetLightingAsRGBChannels() const
{
	int outdoorLightingRaw = GetOutdoorLight();
	int indoorLightingRaw = GetIndoorLight();

	float outdoorLighting = RangeMapFloat((float) outdoorLightingRaw, 0.f, (float)BLOCK_MAX_LIGHTING, 0.f, 1.0f);
	float indoorLighting = RangeMapFloat((float) indoorLightingRaw, 0.f, (float)BLOCK_MAX_LIGHTING, 0.f, 1.0f);

	return Rgba(outdoorLighting, indoorLighting, 0.5f, 1.0f);
}


//-----------------------------------------------------------------------------------------------
// Sets whether this block is part of the sky or not
//
inline void	Block::SetIsPartOfSky(bool isPartOfSky)
{
	if (isPartOfSky)
	{
		m_flags |= BLOCK_BIT_IS_SKY;
	}
	else
	{
		m_flags &= ~BLOCK_BIT_IS_SKY;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets whether the lighting on this block is dirty and needs to be updated
//
inline void	Block::SetIsLightingDirty(bool isLightingDirty)
{
	if (isLightingDirty)
	{
		m_flags |= BLOCK_BIT_IS_LIGHT_DIRTY;
	}
	else
	{
		m_flags &= ~BLOCK_BIT_IS_LIGHT_DIRTY;
	}
}

//-----------------------------------------------------------------------------------------------
// Sets whether this block is fully opaque
//
inline void	Block::SetIsFullyOpaque(bool isFullyOpaque)
{
	if (isFullyOpaque)
	{
		m_flags |= BLOCK_BIT_IS_FULLY_OPAQUE;
	}
	else
	{
		m_flags &= ~BLOCK_BIT_IS_FULLY_OPAQUE;
	}
}

//-----------------------------------------------------------------------------------------------
// Sets whether this block is visible or not
//
inline void	Block::SetIsVisible(bool isVisible)
{
	if (isVisible)
	{
		m_flags |= BLOCK_BIT_IS_VISIBLE;
	}
	else
	{
		m_flags &= ~BLOCK_BIT_IS_VISIBLE;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets whether this block is solid or not
//
inline void	Block::SetIsSolid(bool isSolid)
{
	if (isSolid)
	{
		m_flags |= BLOCK_BIT_IS_SOLID;
	}
	else
	{
		m_flags &= ~BLOCK_BIT_IS_SOLID;
	}
}

//-----------------------------------------------------------------------------------------------
// Sets the outdoor lighting for this block
//
inline void	Block::SetOutdoorLighting(int outdoorLightValue)
{
	outdoorLightValue = ClampInt(outdoorLightValue, 0, BLOCK_MAX_LIGHTING);
	int outdoorValueShifted = (outdoorLightValue << 4);
	
	// Clear the old value
	m_light &= ~BLOCK_OUTDOOR_LIGHT_MASK;

	// Apply the new value
	m_light |= outdoorValueShifted;
}


//-----------------------------------------------------------------------------------------------
// Sets the indoor lighting for this block
//
inline void	Block::SetIndoorLighting(int indoorLightValue)
{
	indoorLightValue = ClampInt(indoorLightValue, 0, BLOCK_MAX_LIGHTING);

	// Clear the old value
	m_light &= ~BLOCK_INDOOR_LIGHT_MASK;

	// Apply the new value
	m_light |= indoorLightValue;
}
