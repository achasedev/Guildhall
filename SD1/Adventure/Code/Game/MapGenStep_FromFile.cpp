/************************************************************************/
/* File: MapGenStep_FromFile.cpp
/* Author: Andrew Chase
/* Date: November 18th, 2017
/* Bugs: None
/* Description: Implementation of the MapGenStep_FromFile class
/************************************************************************/
#include "Game/MapGenStep_FromFile.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

// Static map to track loaded images
std::map<std::string, Image*> MapGenStep_FromFile::s_images;


//-----------------------------------------------------------------------------------------------
// Constructor - parses a FromFile generation step element for the data and initializes accordingly
//
MapGenStep_FromFile::MapGenStep_FromFile(const tinyxml2::XMLElement& genStepXmlElement)
	: MapGenStep(genStepXmlElement)
{
	std::string fileName = ParseXmlAttribute(genStepXmlElement, "fileName", nullptr);

	// Check if the given file was already loaded
	bool fileAlreadyLoaded = (s_images.find(fileName) != s_images.end());

	if (fileAlreadyLoaded)
	{
		m_image = s_images[fileName];
	}
	else
	{
		// Load the image
		Image* loadedImage = new Image(Stringf("Data/Images/%s",fileName.c_str()));
		
		m_image = loadedImage;
		s_images[fileName] = loadedImage;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor - currently unused
//
MapGenStep_FromFile::~MapGenStep_FromFile()
{
}


//-----------------------------------------------------------------------------------------------
// Override of virtual function from class MapGenStep
// Checks the chance to run and number of iterations, and runs the FromFile generation accordingly
//
void MapGenStep_FromFile::Run(Map& mapToAugment)
{
	bool shouldRunStep = CheckRandomChance(m_chanceToRun);

	if (shouldRunStep)
	{
		int numIterations = m_iterationsRange.GetRandomInRange();

		for (int i = 0; i < numIterations; i++)
		{
			ApplyFromFile(mapToAugment);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Reads the file data (Rgba objects) and converts it to TileDefinitions, then alters the map
// by assigning definitions to the corresponding tiles
//
void MapGenStep_FromFile::ApplyFromFile(Map& mapToAugment) const
{
	IntVector2 imageDimensions = m_image->GetDimensions();
	IntVector2 mapDimensions = mapToAugment.GetDimensions();

	// Place the image in the bottom left of the map
	for (int mapXIndex = 0; mapXIndex < mapDimensions.x; mapXIndex++)
	{
		for (int mapYIndex = 0; mapYIndex < mapDimensions.y; mapYIndex++)
		{
			int imageXIndex = mapXIndex;
			int imageYIndex = imageDimensions.y - mapYIndex - 1;

			// If we're iterating over tiles that aren't in the image anymore, continue
			if (!ImageCoordsInRange(imageXIndex, imageYIndex, imageDimensions)) { continue; }

			// Get the definition associated with the image color texel
			Rgba currImageColor = m_image->GetTexel(imageXIndex, imageYIndex);
			const TileDefinition* definitionFromImageColor = TileDefinition::GetTileDefinitionFromColor(currImageColor);

			// Use alpha as a percent chance to change this tile
			float chance = (static_cast<float>(currImageColor.a) / 255.f);

			if (CheckRandomChance(chance))
			{
				// Change the tile
				Tile* currTile = mapToAugment.GetTileFromCoords(IntVector2(mapXIndex, mapYIndex));
				currTile->SetDefinition(definitionFromImageColor);
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if the image coordinates constructed from map indices are within range
//
bool MapGenStep_FromFile::ImageCoordsInRange(int xIndex, int yIndex, const IntVector2& dimensions) const
{
	bool xInRange = (xIndex >= 0 && xIndex < dimensions.x);
	bool yInRange = (yIndex >= 0 && yIndex < dimensions.y);

	return (xInRange && yInRange);
}
