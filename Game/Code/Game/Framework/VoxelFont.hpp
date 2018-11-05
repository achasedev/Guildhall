/************************************************************************/
/* File: VoxelFont.hpp
/* Author: Andrew Chase
/* Date: November 3rd 2018
/* Description: Class to represent a font drawn in a VoxelGrid
/************************************************************************/
#pragma once
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Rendering/Resources/VoxelTexture.hpp"
#include <map>
#include <string>

class Image;

class VoxelFont
{
public:
	//-----Public Methods-----
	
	VoxelFont(const std::string& name, const std::string& imageFile);

	Rgba GetColorForGlyphPixel(const char glyph, const IntVector2& offset) const;

	// Producers
	IntVector3 GetGlyphDimensions() const;
	IntVector3 GetTextDimensions(const std::string& text) const;


public:
	//-----Public Data-----
	


private:
	//-----Private Methods-----
	
	

private:
	//-----Private Data-----
	
	std::string m_name;
	Image* m_image = nullptr;
	const IntVector2 m_glyphLayout;
};
