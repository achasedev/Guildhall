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

#include "Game/Framework/Game.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

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

	GUARANTEE_OR_DIE(image != nullptr, Stringf("Error: Map::Initialize couldn't load height map file \"%s\"", filepath.c_str()));

	IntVector2 imageDimensions = image->GetTexelDimensions();
	m_mapCellLayout = IntVector2(imageDimensions.x - 1, imageDimensions.y - 1);

	// Assertions
	// Each texel of the image is a vertex, so ensure the image is at least 2x2
	GUARANTEE_OR_DIE(imageDimensions.x >= 2 && imageDimensions.y >= 2, Stringf("Error: Map::Initialize received bad map image \"%s\"", filepath.c_str()));

	// Assert that the image dimensions can be evenly chunked
	GUARANTEE_OR_DIE(((imageDimensions.x - 1) % chunkLayout.x == 0) && ((imageDimensions.y - 1) % chunkLayout.y == 0), Stringf("Error: Map::Initialize couldn't match the chunk layout to the image \"%s\"", filepath.c_str()));

	// Build mesh as chunks
	BuildTerrain(image);

	delete image;
}


//-----------------------------------------------------------------------------------------------
// Returns the height at the map at the position of the vertex given by vertexCoord
//
float Map::GetHeightAtVertexCoord(const IntVector2& vertexCoord)
{
	if (vertexCoord.x < 0 || vertexCoord.x >= m_mapCellLayout.x || vertexCoord.y < 0 || vertexCoord.y >= m_mapCellLayout.y)
	{
		return 0.f;
	}

	int index = (m_mapCellLayout.x + 1) * vertexCoord.y + vertexCoord.x;

	return m_mapVertices[index].position.y;
}


//-----------------------------------------------------------------------------------------------
// Returns the height at the map at the given position
//
float Map::GetHeightAtPosition(const Vector3& position)
{
	if (!IsPositionInCellBounds(position))
	{
		return 0.f;
	}

	Vector2 normalizedMapCoords = RangeMap(position.xz(), m_worldBounds.mins, m_worldBounds.maxs, Vector2::ZERO, Vector2::ONES);
	Vector2 cellCoords = Vector2(normalizedMapCoords.x * m_mapCellLayout.x, normalizedMapCoords.y * m_mapCellLayout.y);

	// Flip texel coords since image is top left (0,0)
	cellCoords.y = (m_mapCellLayout.y - cellCoords.y - 1);

	IntVector2 texelCoords = IntVector2(cellCoords);
	Vector2 cellFraction = cellCoords - texelCoords.GetAsFloats();

	// Quad we're in
	IntVector2 bli = texelCoords;
	IntVector2 bri = texelCoords + IntVector2(1, 0);
	IntVector2 tli = texelCoords + IntVector2(0, 1);
	IntVector2 tri = texelCoords + IntVector2(1, 1);

	// Get the heights
	float blh = GetHeightAtVertexCoord(bli);
	float brh = GetHeightAtVertexCoord(bri);
	float tlh = GetHeightAtVertexCoord(tli);
	float trh = GetHeightAtVertexCoord(tri);

	// Bilinear interpolate to find the height
	float bottomHeight	= Interpolate(blh, brh, cellFraction.x);
	float topheight		= Interpolate(tlh, trh, cellFraction.x);
	float finalHeight	= Interpolate(bottomHeight, topheight, cellFraction.y);

	return finalHeight;
}


//-----------------------------------------------------------------------------------------------
// Returns the normal at the map at the position of the vertex given by vertexCoord
//
Vector3 Map::GetNormalAtVertexCoord(const IntVector2& vertexCoord)
{
	if (vertexCoord.x < 0 || vertexCoord.x >= m_mapCellLayout.x || vertexCoord.y < 0 || vertexCoord.y >= m_mapCellLayout.y)
	{
		return 0.f;
	}

	int index = (m_mapCellLayout.x + 1) * vertexCoord.y + vertexCoord.x;

	return m_mapVertices[index].normal;
}


//-----------------------------------------------------------------------------------------------
// Returns the normal of the map at the given position
//
Vector3 Map::GetNormalAtPosition(const Vector3& position)
{
	if (!IsPositionInCellBounds(position))
	{
		return Vector3::DIRECTION_UP;
	}

	Vector2 normalizedMapCoords = RangeMap(position.xz(), m_worldBounds.mins, m_worldBounds.maxs, Vector2::ZERO, Vector2::ONES);
	Vector2 cellCoords = Vector2(normalizedMapCoords.x * m_mapCellLayout.x, normalizedMapCoords.y * m_mapCellLayout.y);

	// Flip texel coords since image is top left (0,0)
	cellCoords.y = (m_mapCellLayout.y - cellCoords.y - 1);

	IntVector2 texelCoords = IntVector2(cellCoords);
	Vector2 cellFraction = cellCoords - texelCoords.GetAsFloats();

	// Quad we're in
	IntVector2 bli = texelCoords;
	IntVector2 bri = texelCoords + IntVector2(1, 0);
	IntVector2 tli = texelCoords + IntVector2(0, 1);
	IntVector2 tri = texelCoords + IntVector2(1, 1);

	// Get the normals
	Vector3 blh = GetNormalAtVertexCoord(bli);
	Vector3 brh = GetNormalAtVertexCoord(bri);
	Vector3 tlh = GetNormalAtVertexCoord(tli);
	Vector3 trh = GetNormalAtVertexCoord(tri);

	// Bilinear interpolate to find the height
	Vector3 bottomNormal	= Interpolate(blh, brh, cellFraction.x);
	Vector3 topNormal		= Interpolate(tlh, trh, cellFraction.x);
	Vector3 finalNormal		= Interpolate(bottomNormal, topNormal, cellFraction.y);

	return finalNormal;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given position is in the xz-bounds of the map
//
bool Map::IsPositionInCellBounds(const Vector3& position)
{
	bool inXBounds = (m_worldBounds.mins.x <= position.x && m_worldBounds.maxs.x >= position.x);
	bool inYBounds = (m_worldBounds.mins.y <= position.z && m_worldBounds.maxs.y >= position.z);

	return (inXBounds && inYBounds);
}


//-----------------------------------------------------------------------------------------------
// Performs a Raycast from the given position in the given direction, returning a hit result
// Assumes the raycast starts above the map
//
RaycastHit_t Map::Raycast(const Vector3& startPosition, const Vector3& direction)
{
	float cellWidth = m_worldBounds.GetDimensions().x / (float) m_mapCellLayout.x;
	float cellHeight = m_worldBounds.GetDimensions().y / (float) m_mapCellLayout.y;

	float stepSize = MinFloat(cellHeight, cellWidth);

	float distanceTravelled = 0.f;
	Vector3 lastPosition = startPosition;

	while (distanceTravelled < MAX_RAYCAST_DISTANCE)
	{
		distanceTravelled += stepSize;
		Vector3 offset = direction * distanceTravelled;

		Vector3 currPosition = startPosition + offset;

		// If we're off the map just stop
		if (!IsPositionInCellBounds(currPosition))
		{
			return RaycastHit_t(false);
		}

		float heightOfMap = GetHeightAtPosition(currPosition);

		// Position is under the map, so converge and return the hit
		if (heightOfMap >= currPosition.y)
		{
			return ConvergeRaycast(lastPosition, currPosition);
		}

		lastPosition = currPosition;
	}

	return RaycastHit_t(false);
}


//-----------------------------------------------------------------------------------------------
// Constructs all positions and UVs used by the entire map from the heightmap image
// All positions are defined in world space
//
void Map::ConstructMapVertexList(Image* heightMap)
{
	std::vector<Vector3> positions;
	std::vector<Vector2> uvs;

	CalculateInitialPositionsAndUVs(positions, uvs, heightMap);

	IntVector2	imageDimensions = heightMap->GetTexelDimensions();

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, false);

	for (int texelYIndex = 0; texelYIndex < imageDimensions.y - 1; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x - 1; ++texelXIndex)
		{
			int tl = texelYIndex * imageDimensions.x + texelXIndex;
			int tr = tl + 1;
			int bl = tl + imageDimensions.x;
			int br = bl + 1;

			// Set the vertices for the mesh
			mb.SetUVs(uvs[tl]);
			mb.PushVertex(positions[tl]);

			mb.SetUVs(uvs[bl]);
			mb.PushVertex(positions[bl]);

			mb.SetUVs(uvs[br]);
			mb.PushVertex(positions[br]);

			mb.SetUVs(uvs[tl]);
			mb.PushVertex(positions[tl]);

			mb.SetUVs(uvs[br]);
			mb.PushVertex(positions[br]);

			mb.SetUVs(uvs[tr]);
			mb.PushVertex(positions[tr]);
		}
	}

	mb.FinishBuilding();
	mb.GenerateFlatTBN();

	// Push the MeshBuilder data into the list
	for (int texelYIndex = 0; texelYIndex < imageDimensions.y - 1; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x - 1; ++texelXIndex)
		{
			int index = (texelYIndex * (imageDimensions.x - 1) + texelXIndex) * 6;

			VertexLit litVertex1 = mb.GetVertex<VertexLit>(index);
			MapVertex mapVertex1;

			mapVertex1.position	= litVertex1.m_position;
			mapVertex1.normal	= litVertex1.m_normal;
			mapVertex1.tangent  = litVertex1.m_tangent;
			mapVertex1.uv		= litVertex1.m_texUVs;

			m_mapVertices.push_back(mapVertex1);

			if (texelXIndex == imageDimensions.x - 2)
			{
				index += 5;

				VertexLit litVertex2 = mb.GetVertex<VertexLit>(index);
				MapVertex mapVertex2;

				mapVertex2.position	= litVertex2.m_position;
				mapVertex2.normal	= litVertex2.m_normal;
				mapVertex2.tangent  = litVertex2.m_tangent;
				mapVertex2.uv		= litVertex2.m_texUVs;

				m_mapVertices.push_back(mapVertex2);
			}
		}
	}

	// Grab the last row
	for (int texelXIndex = 0; texelXIndex < imageDimensions.x - 1; ++texelXIndex)
	{
		int y = imageDimensions.y - 2;
		int index = ((y * (imageDimensions.x - 1) + texelXIndex) * 6) + 1;

		VertexLit litVertex1 = mb.GetVertex<VertexLit>(index);
		MapVertex mapVertex1;

		mapVertex1.position	= litVertex1.m_position;
		mapVertex1.normal	= litVertex1.m_normal;
		mapVertex1.tangent  = litVertex1.m_tangent;
		mapVertex1.uv		= litVertex1.m_texUVs;

		m_mapVertices.push_back(mapVertex1);

		if (texelXIndex == imageDimensions.x - 2)
		{
			index++;

			VertexLit litVertex2 = mb.GetVertex<VertexLit>(index);
			MapVertex mapVertex2;

			mapVertex2.position	= litVertex2.m_position;
			mapVertex2.normal	= litVertex2.m_normal;
			mapVertex2.tangent  = litVertex2.m_tangent;
			mapVertex2.uv		= litVertex2.m_texUVs;

			m_mapVertices.push_back(mapVertex2);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Calculates the positions and uvs for each vertex in the map
//
void Map::CalculateInitialPositionsAndUVs(std::vector<Vector3>& positions, std::vector<Vector2>& uvs, Image* image)
{
	IntVector2	imageDimensions = image->GetTexelDimensions();
	Vector2		worldDimensions = m_worldBounds.GetDimensions();

	float xStride = worldDimensions.x / (float) (imageDimensions.x - 1);
	float zStride = worldDimensions.y / (float) (imageDimensions.y - 1);

	for (int texelYIndex = 0; texelYIndex < imageDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x; ++texelXIndex)
		{
			// Determine the UV
			float u = ((float) texelXIndex / ((float) imageDimensions.x - 1));
			float v = 1.0f - ((float) texelYIndex / ((float) imageDimensions.y - 1));

			Vector2 uv = Vector2(u, v);
			uvs.push_back(uv);

				// Determine the position
			float x = m_worldBounds.mins.x + texelXIndex * xStride;
			float z = m_worldBounds.maxs.y - texelYIndex * zStride;
			float y = image->GetTexelGrayScale(texelXIndex, texelYIndex);
			y = RangeMapFloat(y, 0.f, 1.f, m_heightRange.min, m_heightRange.max);

			Vector3 position = Vector3(x, y, z);
			positions.push_back(position);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Builds the mesh of the map by building all the individual chunks
//
void Map::BuildTerrain(Image* heightMap)
{
	// Construct all map vertices and heights, and store them in a collection
	ConstructMapVertexList(heightMap);

	// Set up the material for the map
	Material* mapMaterial = AssetDB::GetSharedMaterial("Data/Materials/Map.material");

	// Across chunks - y
	for (int chunkYIndex = 0; chunkYIndex < m_chunkLayout.y; ++chunkYIndex)
	{
		// Across chunks - x
		for (int chunkXIndex = 0; chunkXIndex < m_chunkLayout.x; ++chunkXIndex)
		{
			// Build this chunk
			BuildSingleChunk(chunkXIndex, chunkYIndex, mapMaterial);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Builds a single chunk renderable, given the information from the height map and the index of the current chunk
//
void Map::BuildSingleChunk(int chunkXIndex, int chunkYIndex, Material* material)
{
	// Set up initial state variables
	IntVector2 chunkDimensions = IntVector2(m_mapCellLayout.x / m_chunkLayout.x, m_mapCellLayout.y / m_chunkLayout.y);

	int texelXStart = chunkXIndex * chunkDimensions.x;
	int texelYStart = chunkYIndex * chunkDimensions.y;

	// Iterate across all texels corresponding to this chunk and get the positions
	std::vector<Vector3> chunkPositions;

	for (int texelYIndex = texelYStart; texelYIndex <= texelYStart + chunkDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = texelXStart; texelXIndex <= (texelXStart + chunkDimensions.x); ++texelXIndex)
		{
			int vertexIndex = texelYIndex * (m_mapCellLayout.x + 1) + texelXIndex;
			chunkPositions.push_back(m_mapVertices[vertexIndex].position);	
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
			int tl = texelYIndex * (m_mapCellLayout.x + 1) + texelXIndex;
			int tr = tl + 1;
			int bl = tl + (m_mapCellLayout.x + 1);
			int br = bl + 1;

			// Transform them to local space (local to this specific chunk)
			Vector3 tlPosition = toLocal.TransformPoint(m_mapVertices[tl].position).xyz();
			Vector3 trPosition = toLocal.TransformPoint(m_mapVertices[tr].position).xyz();
			Vector3 blPosition = toLocal.TransformPoint(m_mapVertices[bl].position).xyz();
			Vector3 brPosition = toLocal.TransformPoint(m_mapVertices[br].position).xyz();

			// Set the vertices for the mesh
			mb.SetUVs(m_mapVertices[tl].uv);
			mb.SetNormal(m_mapVertices[tl].normal);
			mb.SetTangent(m_mapVertices[tl].tangent);
			mb.PushVertex(tlPosition);

			mb.SetUVs(m_mapVertices[bl].uv);
			mb.SetNormal(m_mapVertices[bl].normal);
			mb.SetTangent(m_mapVertices[bl].tangent);
			mb.PushVertex(blPosition);

			mb.SetUVs(m_mapVertices[br].uv);
			mb.SetNormal(m_mapVertices[br].normal);
			mb.SetTangent(m_mapVertices[br].tangent);
			mb.PushVertex(brPosition);

			mb.SetUVs(m_mapVertices[tl].uv);
			mb.SetNormal(m_mapVertices[tl].normal);
			mb.SetTangent(m_mapVertices[tl].tangent);
			mb.PushVertex(tlPosition);

			mb.SetUVs(m_mapVertices[br].uv);
			mb.SetNormal(m_mapVertices[br].normal);
			mb.SetTangent(m_mapVertices[br].tangent);
			mb.PushVertex(brPosition);

			mb.SetUVs(m_mapVertices[tr].uv);
			mb.SetNormal(m_mapVertices[tr].normal);
			mb.SetTangent(m_mapVertices[tr].tangent);
			mb.PushVertex(trPosition);
		}
	}

	// Generate the mesh with normals and tangents
	mb.FinishBuilding();
	Mesh* chunkMesh = mb.CreateMesh();

	MapChunk* chunk = new MapChunk(model, chunkMesh, material);
	m_mapChunks.push_back(chunk);
}


//-----------------------------------------------------------------------------------------------
// Determines the hit of a Raycast given the positions before and after when the hit occurred
//
RaycastHit_t Map::ConvergeRaycast(Vector3& positionBeforeHit, Vector3& positionAfterhit)
{
	Vector3 midpoint;
	for (int iteration = 0; iteration < RAYCAST_CONVERGE_ITERATION_COUNT; ++iteration)
	{
		midpoint = (positionAfterhit + positionBeforeHit) * 0.5f;

		// Check for early out
		float mapHeight = GetHeightAtPosition(midpoint);
		Vector3 mapPosition = Vector3(midpoint.x, mapHeight, midpoint.z);
		float distance = (mapPosition - midpoint).GetLength();

		if (distance < RAYCAST_CONVERGE_EARLYOUT_DISTANCE)
		{
			return RaycastHit_t(true, mapPosition);
		}

		// No early out, so update the endpositions and continue iterating
		if (midpoint.y > mapPosition.y)
		{
			// Still above map, so update start
			positionBeforeHit = midpoint;
		}
		else
		{
			positionAfterhit = midpoint;
		}
	}

	// Didn't fully converged, so just return our last midpoint
	return RaycastHit_t(true, midpoint);
}
