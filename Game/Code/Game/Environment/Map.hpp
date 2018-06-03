#pragma once

#include <string>
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"

class Image;
class MapChunk;
class Renderable;

class Map
{
public:
	//-----Public Methods-----

	Map() {}
	~Map();

	void Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& fileName);

	Image* GetImage() const;

private:
	//-----Private Methods-----

private:
	//-----Private Data-----

	AABB2 m_worldBounds;
	IntVector2 m_dimensions;
	FloatRange m_heightRange;
	IntVector2 m_chunkLayout;
	std::vector<MapChunk*> m_mapChunks;

	Image* m_image;

};
