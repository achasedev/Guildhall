/************************************************************************/
/* File: VoxelFont.hpp
/* Author: Andrew Chase
/* Date: November 3rd 2018
/* Description: Class to represent a font drawn in a VoxelGrid
/************************************************************************/
#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVector3.hpp"
#include <string>

class Image;

class VoxelFont
{
public:
	//-----Public Methods-----
	
	VoxelFont(const std::string& name, const std::string& imageFile);

	// For drawing to the grid
	Rgba GetColorForGlyphPixel(const char glyph, const IntVector2& offset) const;

	// Producers
	IntVector3 GetGlyphDimensions() const;
	IntVector3 GetTextDimensions(const std::string& text) const;


private:
	//-----Private Data-----
	
	std::string m_name;
	Image* m_image = nullptr;		// Monospace, 16x16 glyph font png
	const IntVector2 m_glyphLayout; // Always 16x16

};


//-----------------------------------------------------------------------------------------------
// Returns an offset to be applied to the current voxel at local coords when rendering fonts
//
IntVector3 GetOffsetForFontWaveEffect(const IntVector3& textDimensions, const IntVector3& localCoords);