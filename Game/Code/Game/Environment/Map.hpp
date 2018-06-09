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
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image;
class Vector2;
class Vector3;
class Material;
class MapChunk;
class Renderable;

struct MapVertex
{
	Vector3 position;
	Vector3 normal;
	Vector4 tangent;
	Vector2 uv;
};


class Map
{
public:
	//-----Public Methods-----

	Map() {}
	~Map();

	void Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName);

	// Accessors
	float GetHeightAtVertexCoord(const IntVector2& vertexCoord);
	float GetHeightAtPosition(const Vector3& position);
	
	// Producers
	bool IsPositionInCellBounds(const Vector3& position);


private:
	//-----Private Methods-----

	void BuildTerrain(Image* heightMap);
		void ConstructMapVertexList(Image* heightMap);
			void CalculateInitialPositionsAndUVs(std::vector<Vector3>& positions, std::vector<Vector2>& uvs, Image* image);
		void BuildSingleChunk(int chunkXIndex, int chunkYIndex, Material* material);


private:
	//-----Private Data-----

	AABB2					m_worldBounds;		// World-unit boundary
	IntVector2				m_mapCellLayout;	// Texel/Cell dimensions
	IntVector2				m_chunkLayout;		// Number of chunks wide/long

	FloatRange				m_heightRange;		// Max/Min height for the map
	std::vector<MapChunk*>	m_mapChunks;		// List of chunks

	std::vector<MapVertex>	m_mapVertices;
};
