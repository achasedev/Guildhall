#pragma once
#include "Game/Environment/Block.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include <vector>

class Renderable;

class Terrain
{
public:
	//-----Public Methods-----

	Terrain();
	~Terrain();

	void LoadFromFile(const char* filename);

	// Accessors

	Block& GetBlockAtCoordinate(int x, int y, int z);
	Block& GetBlockAtIndex(unsigned int index);

	Renderable* GetRenderable() const;

private:
	//-----Private Data-----

	IntVector3	m_blockDimensions;	// Dimensions of the terrain in blocks
	AABB3		m_worldBounds;		// Extents of the terrain in world units, mins should *always* be (0,0,0)

	Renderable* m_renderable;
	std::vector<Block> m_blocks;	// Blocks that construct the basic terrain, linear array in order of x, y, then z
};
