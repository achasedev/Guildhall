/************************************************************************/
/* File: MapGenStep_FromFile.hpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Reads a color-representative image from file, converts
/*				the colors to TileDefinitions, and sets the tiles in the map
/*				to those given definitions
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image;


class MapGenStep_FromFile : public MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep_FromFile( const tinyxml2::XMLElement& genStepXmlElement );
	virtual ~MapGenStep_FromFile() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void ApplyFromFile(Map& mapToAugment) const;

	bool ImageCoordsInRange(int xIndex, int yIndex, const IntVector2& dimensions) const;	// Ensure we don't go outside the image

private:
	//-----Private Data-----
	
	Image* m_image;										// The texel image to load in
	static std::map<std::string, Image*> s_images;		// Static map of images loaded, to prevent multiple loads of a single file
};
