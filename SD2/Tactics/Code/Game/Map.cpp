/************************************************************************/
/* File: Map.cpp
/* Author: Andrew Chase
/* Date: February 17th, 2017
/* Description: Implementation of the Map class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/HeatMap.hpp"

// Statics
const IntVector3 Map::MAP_STEP_NORTH	= IntVector3( 0,  1, 0);
const IntVector3 Map::MAP_STEP_SOUTH	= IntVector3( 0, -1, 0);
const IntVector3 Map::MAP_STEP_EAST		= IntVector3( 1,  0, 0);
const IntVector3 Map::MAP_STEP_WEST		= IntVector3(-1,  0, 0);

//-----------------------------------------------------------------------------------------------
// Default constructor
//
Map::Map()
	: m_heightMap(nullptr)
	, m_dimensions(IntVector3::ZERO)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Map::~Map()
{
	delete m_heightMap;
	m_heightMap = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the map block environment given a height map image
//
bool Map::LoadFromHeightMap(const Image& heights, unsigned int maxHeight /*= 8*/)
{
	// Set up the height map
	IntVector2 mapDimensions = heights.GetDimensions();
	m_heightMap = new HeatMap(mapDimensions, (float)maxHeight);

	for (int y = 0; y < mapDimensions.y; ++y)
	{
		for (int x = 0; x < mapDimensions.x; ++x)
		{
			float grayscale = (float) heights.GetTexelGrayScale(x, y);
			float heightValue = RangeMapFloat(grayscale, 0.f, 255.f, 0.f, (float) (maxHeight - 1));
			heightValue = (float) RoundToNearestInt(heightValue);
			m_heightMap->SetHeat(IntVector2(x, y), heightValue);
		}
	}

	// Construct the map from the heightmap
	m_dimensions = IntVector3(mapDimensions.x, mapDimensions.y, maxHeight);
	m_blocks.resize(GetNumBlocks());

	for (int y = 0; y < mapDimensions.y; ++y)
	{
		for (int x = 0; x < mapDimensions.x; ++x)
		{
			int height = GetHeightAtMapCoords(IntVector2(x, y));
			for (int z = 0; z < height; ++z)
			{
				GetBlock(IntVector3(x, y, z))->SetType(BlockDefinition::GetBlockDefinitionByName("Grass"));
			}
		}
	}
	return true;
}


//-----------------------------------------------------------------------------------------------
// Constructs the map block environment given a file name for a heightmap image
//
bool Map::LoadFromHeightMap(const char* filename, unsigned int maxHeight)
{
	Image* heightImage = new Image(filename);
	heightImage->FlipVertical();
	return LoadFromHeightMap(*heightImage, maxHeight);
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Map::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Draws the map to screen
//
void Map::Render()
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentShaderProgram("Block");
	Vertex3D_PCU* vertices = (Vertex3D_PCU*) malloc(sizeof(Vertex3D_PCU) * GetNumBlocks() * 24);
	int vertexOffset = 0;
	unsigned int* indices = (unsigned int*) malloc(sizeof(int) * GetNumBlocks() * 36);
	int indexOffset = 0;

	// Iterate across all blocks to assemble the vertex array
	for (int zIndex = 0; zIndex < m_dimensions.z; ++zIndex)
	{
		for (int yIndex = 0; yIndex < m_dimensions.y; ++yIndex)
		{
			for (int xIndex = 0; xIndex < m_dimensions.x; ++xIndex)
			{
				// Get the block and definition at this index
				IntVector3 currCoords = IntVector3(xIndex, yIndex, zIndex);
				BlockDefinition* currDef = GetBlock(currCoords)->GetType();

				// If this block isn't air, assemble it's vertices and add them to the vertex collection
				if (currDef != nullptr)
				{
					Vector3 centerPos = MapCoordsToWorldPosition(currCoords) + Vector3(0.f, 0.5f, 0.f);
					renderer->AppendCubeVertices(&vertices[0], vertexOffset, &indices[0], indexOffset, centerPos, Vector3::ONES, Rgba::WHITE, currDef->GetTopUVs(), currDef->GetSideUVs(), currDef->GetBottomUVs());
				}
			}
		}
	}

	// Set up the renderer
	renderer->ClearScreen(Rgba::BLACK);
	renderer->BindTexture(0, BlockDefinition::GetBlockTextureAtlas()->GetTexture().GetHandle());

	renderer->EnableDepth(COMPARE_LESS, true);
	renderer->ClearDepth();
	
	// Set up the outline
	Texture* blockTexture = renderer->CreateOrGetTexture("Data/Images/BlockBorder.png");
	renderer->BindTexture(1, blockTexture->GetHandle());

	// Draw the blocks
	renderer->DrawMeshImmediate(vertices, vertexOffset, PRIMITIVE_TRIANGLES, indices, indexOffset);

	renderer->SetCurrentShaderProgram("Default");
	free(vertices);
	free(indices);
}


//-----------------------------------------------------------------------------------------------
// Returns the index for the block given by coords
//
int Map::GetBlockIndex(const IntVector3& coords) const
{
	if (!Are3DCoordsValid(coords))
	{
		return -1;
	}

	int numBlocksInLayer = m_dimensions.x * m_dimensions.y;
	int index = numBlocksInLayer * coords.z + m_dimensions.x * coords.y + coords.x;

	return index;
}


//-----------------------------------------------------------------------------------------------
// Checks if the 2D coords are within the map dimensions
//
bool Map::Are2DCoordsValid(const IntVector3& coords) const
{
	if (coords.x < 0 || coords.x >= m_dimensions.x) { return false; }
	if (coords.y < 0 || coords.y >= m_dimensions.y) { return false; }

	return true;
}


//-----------------------------------------------------------------------------------------------
// Checks if the 3D coords are within the map dimensions
//
bool Map::Are3DCoordsValid(const IntVector3& coords) const
{
	if (coords.x < 0 || coords.x >= m_dimensions.x) { return false; }
	if (coords.y < 0 || coords.y >= m_dimensions.y) { return false; }
	if (coords.z < 0 || coords.z >= m_dimensions.z)	{ return false; }

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the position is inside a solid block, false otherwise
//
bool Map::IsPositionInSolidBlock(const Vector3& position) const
{
	IntVector3 coords = WorldPositionToMapCoords(position);

	// If the position is off the map then return false
	if (Are3DCoordsValid(coords))
	{
		// Valid coords, so get the block
		int index = GetBlockIndex(coords);

		if (m_blocks.at(index).GetType() != nullptr)
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns the block coordinate that contains the given position
//
IntVector3 Map::WorldPositionToMapCoords(const Vector3& position) const
{
	IntVector3 mapCoord;

	mapCoord.x = (int) position.x;
	mapCoord.y = (int) position.z;
	mapCoord.z = (int) position.y;

	return mapCoord;
}


//-----------------------------------------------------------------------------------------------
// Returns the block at map coords coords
//
Block* Map::GetBlock(const IntVector3& coords)
{
	if (!Are3DCoordsValid(coords))
	{
		return nullptr;
	}

	int index = GetBlockIndex(coords);
	if (index == -1)
	{
		return nullptr;
	}
	return &m_blocks[index];
}


//-----------------------------------------------------------------------------------------------
// Sets the type of the block at coords to the definition given by definitionName
//
void Map::SetBlockDefinitionByName(const IntVector3& coords, const char* definitionName)
{
	Block* block = GetBlock(coords);
	if (block != nullptr)
	{
		block->SetType(BlockDefinition::GetBlockDefinitionByName(definitionName));
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the height map
//
HeatMap* Map::GetHeightMap() const
{
	return m_heightMap;
}


//-----------------------------------------------------------------------------------------------
// Returns the height (number of solid blocks) at the (x,y) blockPosition
// Also corresponds to the index of the first air block in that position
//
int Map::GetHeightAtMapCoords(const IntVector2& blockPosition) const
{
	return (int) m_heightMap->GetHeat(blockPosition) + 1;	// We add one to correspond with the count of blocks
															// at these coords (i.e. index of first air block)
}


//-----------------------------------------------------------------------------------------------
// Returns the map coords of the block at the given direction
// Sets the z to -1 if the neighbor is off the map
//
IntVector3 Map::GetNeighborCoords(const IntVector3& startCoords, const IntVector2& stepDirection) const
{
	IntVector3 neighborCoords;

	neighborCoords.x = startCoords.x + stepDirection.x;
	neighborCoords.y = startCoords.y + stepDirection.y;

	if (Are3DCoordsValid(neighborCoords))
	{
		neighborCoords.z = GetHeightAtMapCoords(neighborCoords.xy());
	}

	return neighborCoords;
}


//-----------------------------------------------------------------------------------------------
// converts the map coords to the world position of the block position (center of the block's bottom
// face)
//
Vector3 Map::MapCoordsToWorldPosition(const IntVector3& coords) const
{
	Vector3 worldPos;
	worldPos.x = (float) coords.x + 0.5f;	// Offset to indicate the bottom center of the block
	worldPos.y = (float) coords.z;
	worldPos.z = (float) coords.y + 0.5f;	// Offset

	return worldPos;
}


//-----------------------------------------------------------------------------------------------
// Converts a map-space vector to a world-space vector (swaps y and z)
//
Vector3 Map::MapVectorToWorldVector(const IntVector3& mapVector) const
{
	Vector3 worldVector;

	worldVector.x = (float) mapVector.x;
	worldVector.y = (float) mapVector.z;
	worldVector.z = (float) mapVector.y;

	return worldVector;
}


//-----------------------------------------------------------------------------------------------
// Converts a map xy-plane 2D angle to an xz-plane 2D world direction
//
Vector3 Map::MapAngleToWorldDirection(float angle) const
{
	Vector2 direction = Vector2::MakeDirectionAtDegrees(angle);

	Vector3 finalDirection = Vector3(direction.x, 0.f, direction.y);

	return finalDirection;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the two 3D map coordinates correspond to the same xy space (i.e. same column)
//
bool Map::AreSame2DCoordinate(const IntVector3& a, const IntVector3& b)
{
	return (a.x == b.x && a.y == b.y);
}
