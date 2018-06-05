/************************************************************************/
/* File: Map.cpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Implementation of the map class
/************************************************************************/
#include "Game/Environment/Map.hpp"
#include "Game/Environment/MapChunk.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"

//-----------------------------------------------------------------------------------------------
// Destructor - deletes the reference image and all chunks
//
Map::~Map()
{
	// Delete all chunks
	for (int index = 0; index < (int) m_mapChunks.size(); ++index)
	{
		delete m_mapChunks[index];
	}

	m_mapChunks.clear();

	delete m_image;
	m_image = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the mesh chunks for the map from the given height map file path
//
void Map::Intialize(const AABB2& worldBounds, float minHeight, float maxHeight, const IntVector2& chunkLayout, const std::string& filepath)
{
	// Set member variables
	m_worldBounds = worldBounds;
	m_chunkLayout = chunkLayout;
	m_heightRange = FloatRange(minHeight, maxHeight);

	Image* image = AssetDB::CreateOrGetImage(filepath);
	m_image = image;

	IntVector2 imageDimensions = image->GetTexelDimensions();
	m_texelDimensions = IntVector2(imageDimensions.x - 1, imageDimensions.y - 1);

	// Assertions
	// Each texel of the image is a vertex, so ensure the image is at least 2x2
	GUARANTEE_OR_DIE(imageDimensions.x >= 2 && imageDimensions.y >= 2, Stringf("Error: Map::Initialize received bad map image \"%s\"", filepath.c_str()));

	// Assert that the image dimensions can be evenly chunked
	GUARANTEE_OR_DIE(((imageDimensions.x - 1) % chunkLayout.x == 0) && ((imageDimensions.y - 1) % chunkLayout.y == 0), Stringf("Error: Map::Initialize couldn't match the chunk layout to the image \"%s\"", filepath.c_str()));

	// Build chunks
	BuildChunks();
}


//-----------------------------------------------------------------------------------------------
// Returns the height at the map at the given position
//
float Map::GetHeightAtPosition(const Vector3& position)
{
	if (!IsPositionInXZBounds(position))
	{
		return 0.f;
	}

	Vector2 uvs = RangeMap(position.xz(), m_worldBounds.mins, m_worldBounds.maxs, Vector2::ZERO, Vector2::ONES);
	Vector2 mapCoords = Vector2(uvs.x * (float) m_texelDimensions.x, uvs.y * (float) m_texelDimensions.y);

	IntVector2 cellCoords = IntVector2(mapCoords);
	Vector2 cellFraction = mapCoords - cellCoords.GetAsFloats();

	// Quad we're in
	IntVector2 bli = cellCoords;
	IntVector2 bri = cellCoords + IntVector2(1, 0);
	IntVector2 tli = cellCoords + IntVector2(0, 1);
	IntVector2 tri = cellCoords + IntVector2(1, 1);

	// Get the heights
	float blh = m_image->GetTexelGrayScale(bli.x, bli.y);
	float brh = m_image->GetTexelGrayScale(bri.x, bri.y);
	float tlh = m_image->GetTexelGrayScale(tli.x, tli.y);
	float trh = m_image->GetTexelGrayScale(tri.x, tri.y);

	// Bilinear interpolate to find the height
	float bottomHeight	= Interpolate(blh, brh, cellFraction.x);
	float topheight		= Interpolate(tlh, trh, cellFraction.x);
	float finalHeight	= Interpolate(bottomHeight, topheight, cellFraction.y);

	return finalHeight;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given position is in the xz-bounds of the map
//
bool Map::IsPositionInXZBounds(const Vector3& position)
{
	bool inXBounds = (m_worldBounds.mins.x <= position.x && m_worldBounds.maxs.x >= position.x);
	bool inYBounds = (m_worldBounds.mins.y <= position.z && m_worldBounds.maxs.y >= position.z);

	return (inXBounds && inYBounds);
}


//-----------------------------------------------------------------------------------------------
// Constructs all positions and UVs used by the entire map from the heightmap image
// All positions are defined in world space
//
void Map::ConstructPositionAndUVLists(std::vector<Vector3>& positions, std::vector<Vector2>& uvs)
{
	IntVector2 imageDimensions = m_image->GetTexelDimensions();
	Vector2 worldDimensions = m_worldBounds.GetDimensions();

	float xStride = worldDimensions.x / (float) (imageDimensions.x - 1);
	float zStride = worldDimensions.y / (float) (imageDimensions.y - 1);

	for (int texelYIndex = 0; texelYIndex < imageDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x; ++texelXIndex)
		{
			// Determine the position
			float x = m_worldBounds.mins.x + texelXIndex * xStride;
			float z = m_worldBounds.maxs.y - texelYIndex * zStride;
			float y = m_image->GetTexelGrayScale(texelXIndex, texelYIndex);
			y = RangeMapFloat(y, 0.f, 1.f, m_heightRange.min, m_heightRange.max);

			Vector3 position = Vector3(x, y, z);
			positions.push_back(position);

			float u = ((float) texelXIndex / ((float) imageDimensions.x - 1));
			float v = 1.0f - ((float) texelYIndex / ((float) imageDimensions.y - 1));

			Vector2 uv = Vector2(u, v);
			uvs.push_back(uv);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh of the map by building all the individual chunks
//
void Map::BuildChunks()
{
	// Construct all the positions and uvs
	std::vector<Vector3> positions;
	std::vector<Vector2> uvs;

	ConstructPositionAndUVLists(positions, uvs);


	// Set up the material for the map
	Material* mapMaterial = AssetDB::GetSharedMaterial("Map");

	// Across chunks - y
	for (int chunkYIndex = 0; chunkYIndex < m_chunkLayout.y; ++chunkYIndex)
	{
		// Across chunks - x
		for (int chunkXIndex = 0; chunkXIndex < m_chunkLayout.x; ++chunkXIndex)
		{
			// Build this chunk
			BuildSingleChunk(positions, uvs, chunkXIndex, chunkYIndex, mapMaterial);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Builds a single chunk renderable, given the information from the height map and the index of the current chunk
//
void Map::BuildSingleChunk(std::vector<Vector3>& positions, std::vector<Vector2>& uvs, int chunkXIndex, int chunkYIndex, Material* material)
{
	// Set up initial state variables
	IntVector2 imageDimensions = m_image->GetTexelDimensions();
	IntVector2 chunkDimensions = IntVector2((imageDimensions.x - 1) / m_chunkLayout.x, (imageDimensions.y - 1) / m_chunkLayout.y);

	int texelXStart = chunkXIndex * chunkDimensions.x;
	int texelYStart = chunkYIndex * chunkDimensions.y;

	// Iterate across all texels corresponding to this chunk and get the positions
	std::vector<Vector3> chunkPositions;

	for (int texelYIndex = texelYStart; texelYIndex <= texelYStart + chunkDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = texelXStart; texelXIndex <= (texelXStart + chunkDimensions.x); ++texelXIndex)
		{
			int vertexIndex = texelYIndex * imageDimensions.x + texelXIndex;
			chunkPositions.push_back(positions[vertexIndex]);	
		}
	}

	// Get the average location of all the positions, for the model matrix
	Vector3 averagePosition = Vector3::ZERO;
	int positionCount = (int) chunkPositions.size();
	for (int index = 0; index < positionCount; ++index)
	{
		averagePosition += chunkPositions[index];
	}
	averagePosition /= (float) positionCount;

	// Construct matrices use to transform the points
	Matrix44 model = Matrix44::MakeModelMatrix(averagePosition, Vector3::ZERO, Vector3::ONES);
	Matrix44 toLocal = Matrix44::MakeModelMatrix(-1.f * averagePosition, Vector3::ZERO, Vector3::ONES);

	// Build the mesh
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, false);

	// Color it red to make adjacent chunks distinct
	if ((chunkXIndex + chunkYIndex) % 2 == 0)
	{
		mb.SetColor(Rgba::RED);
	}

	for (int texelYIndex = texelYStart; texelYIndex < texelYStart + chunkDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = texelXStart; texelXIndex < (texelXStart + chunkDimensions.x); ++texelXIndex)
		{
			int tl = texelYIndex * imageDimensions.x + texelXIndex;
			int tr = tl + 1;
			int bl = tl + imageDimensions.x;
			int br = bl + 1;

			// Transform them to local space (local to this specific chunk)
			Vector3 tlPosition = toLocal.TransformPoint(positions[tl]).xyz();
			Vector3 trPosition = toLocal.TransformPoint(positions[tr]).xyz();
			Vector3 blPosition = toLocal.TransformPoint(positions[bl]).xyz();
			Vector3 brPosition = toLocal.TransformPoint(positions[br]).xyz();

			// Set the vertices for the mesh
			mb.SetUVs(uvs[tl]);
			mb.PushVertex(tlPosition);

			mb.SetUVs(uvs[bl]);
			mb.PushVertex(blPosition);

			mb.SetUVs(uvs[br]);
			mb.PushVertex(brPosition);

			mb.SetUVs(uvs[tl]);
			mb.PushVertex(tlPosition);

			mb.SetUVs(uvs[br]);
			mb.PushVertex(brPosition);

			mb.SetUVs(uvs[tr]);
			mb.PushVertex(trPosition);
		}
	}

	// Generate the mesh with normals and tangents
	mb.FinishBuilding();
	mb.GenerateSmoothNormals();
	

	Mesh* chunkMesh = mb.CreateMesh();

	MapChunk* chunk = new MapChunk(model, chunkMesh, material);
	m_mapChunks.push_back(chunk);
}
