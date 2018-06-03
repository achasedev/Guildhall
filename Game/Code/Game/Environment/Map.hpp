/************************************************************************/
/* File: Map.hpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Class to represent the playable terrain
/************************************************************************/
#pragma once

#include <string>
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image;
class Vector2;
class Vector3;
class Material;
class MapChunk;
class Renderable;

class Map
{
public:
	//-----Public Methods-----

	Map() {}
	~Map();

	void Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName);


private:
	//-----Private Methods-----

	void BuildChunks();
		void ConstructPositionAndUVLists(std::vector<Vector3>& positions, std::vector<Vector2>& uvs);
		void BuildSingleChunk(std::vector<Vector3>& positions, std::vector<Vector2>& uvs, int chunkXIndex, int chunkYIndex, Material* material);


private:
	//-----Private Data-----

	AABB2					m_worldBounds;		// World-unit boundary
	IntVector2				m_texelDimensions;  // Texel dimensions
	FloatRange				m_heightRange;		// Max/Min height for the map
	IntVector2				m_chunkLayout;		// Number of chunks wide/long
	std::vector<MapChunk*>	m_mapChunks;		// List of chunks

	Image*					m_image;	// Height map image

};
