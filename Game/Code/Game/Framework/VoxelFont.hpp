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

class VoxelFont
{
public:
	//-----Public Methods-----
	
	const VoxelTexture* GetImageForGlyph(const char glyph) const;

	// Producers
	IntVector3 GetTextDimensions(const std::string& text) const;

	static void LoadFont(const std::string& filename);
	static VoxelFont* GetFont(const std::string& fontName);


public:
	//-----Public Data-----
	
	VoxelFont(const std::string& name);


private:
	//-----Private Methods-----
	
	

private:
	//-----Private Data-----
	
	std::string m_name;
	VoxelTexture m_textures[256];

	static std::map<std::string, VoxelFont*> s_fonts;

};
