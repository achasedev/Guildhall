/************************************************************************/
/* File: Map.cpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Implementation of the map class
/************************************************************************/
#include "Game/Entity/Bullet.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/NPCSpawner.hpp"
#include "Game/Environment/MapChunk.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Time/ScopedProfiler.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/Material.hpp"


#include "Game/Framework/Game.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

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
	ScopedProfiler sp = ScopedProfiler("Map::Initialize()");
	UNUSED(sp);

	// Set member variables
	m_worldBounds = worldBounds;
	m_chunkLayout = chunkLayout;
	m_heightRange = FloatRange(minHeight, maxHeight);

	Image* image = AssetDB::CreateOrGetImage(filepath);

	GUARANTEE_OR_DIE(image != nullptr, Stringf("Error: Map::Initialize couldn't load height map file \"%s\"", filepath.c_str()));

	IntVector2 imageDimensions = image->GetTexelDimensions();

	m_mapVertexLayout = imageDimensions;
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
// Performs collision checks on entities and fixes their heights and orientations on the map
//
void Map::Update()
{
	UpdateEntities();
	CheckActorActorCollisions();
	CheckProjectilesAgainstActors();
	DeleteObjectsMarkedForDelete();
	UpdateHeightAndOrientationOnMap();
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the map at the given vertex coordinate
//
Vector3 Map::GetPositionAtVertexCoord(const IntVector2& vertexCoord)
{
	// Clamp to edge
	IntVector2 coord = vertexCoord;
	coord.x = ClampInt(coord.x, 0, m_mapVertexLayout.x);
	coord.y = ClampInt(coord.y, 0, m_mapVertexLayout.y);

	int index = coord.y * m_mapVertexLayout.x + coord.x;

	return m_mapVertices[index].position;
}


//-----------------------------------------------------------------------------------------------
// Returns the height at the map at the position of the vertex given by vertexCoord
//
float Map::GetHeightAtVertexCoord(const IntVector2& vertexCoord)
{
	if (vertexCoord.x < 0 || vertexCoord.x >= m_mapVertexLayout.x || vertexCoord.y < 0 || vertexCoord.y >= m_mapVertexLayout.y)
	{
		return 0.f;
	}

	int index = (m_mapVertexLayout.x) * vertexCoord.y + vertexCoord.x;

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
	if (vertexCoord.x < 0 || vertexCoord.x >= m_mapVertexLayout.x || vertexCoord.y < 0 || vertexCoord.y >= m_mapVertexLayout.y)
	{
		return 0.f;
	}

	int index = (m_mapVertexLayout.x) * vertexCoord.y + vertexCoord.x;

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
// Adds the NPC tank to the map's list of object references
//
void Map::AddNPCTank(NPCTank* tank)
{
	m_npcTanks.push_back(tank);
	m_gameEntities.push_back(tank);
}


//-----------------------------------------------------------------------------------------------
// Adds the bullet to the map's list of object references
//
void Map::AddBullet(Bullet* bullet)
{
	m_bullets.push_back(bullet);
	m_gameEntities.push_back(bullet);
}


//-----------------------------------------------------------------------------------------------
// Adds the given spawner to the map's list of entity references
//
void Map::AddSpawner(NPCSpawner* spawner)
{
	m_spawners.push_back(spawner);
	m_gameEntities.push_back(spawner);
}


//-----------------------------------------------------------------------------------------------
// Performs a Raycast from the given position in the given direction, returning a hit result
// Assumes the raycast starts above the map
//
RaycastHit_t Map::Raycast(const Vector3& startPosition, const Vector3& direction, float distance)
{
	float cellWidth = m_worldBounds.GetDimensions().x / (float) m_mapCellLayout.x;
	float cellHeight = m_worldBounds.GetDimensions().y / (float) m_mapCellLayout.y;

	float stepSize = MinFloat(cellHeight, cellWidth);

	float distanceTravelled = 0.f;
	Vector3 lastPosition = startPosition;

	while (distanceTravelled < distance)
	{
		distanceTravelled += stepSize;
		Vector3 offset = direction * distanceTravelled;

		Vector3 rayPosition = startPosition + offset;

		// If we're off the map just send a no hit response
		if (!IsPositionInCellBounds(rayPosition))
		{
			return RaycastHit_t(false, startPosition + 2000.f * direction, true);
		}

		// Check against any entity first	
		for (int objIndex = 0; objIndex < (int) m_gameEntities.size(); ++objIndex)
		{
			float radius = m_gameEntities[objIndex]->GetPhysicsRadius();
			Vector3 objPosition = m_gameEntities[objIndex]->transform.position;

			float distanceSquared = (objPosition - rayPosition).GetLengthSquared();

			if (distanceSquared < radius * radius)
			{
				return ConvergeRaycastOnObject(lastPosition, rayPosition, m_gameEntities[objIndex]);
			}
		}

		// If no hit against entity, check for hit against terrain
		float heightOfMap = GetHeightAtPosition(rayPosition);

		// Position is under the map, so converge and return the hit
		if (heightOfMap >= rayPosition.y)
		{
			return ConvergeRaycastOnTerrain(lastPosition, rayPosition);
		}

		lastPosition = rayPosition;
	}

	return RaycastHit_t(false, startPosition + 2000.f * direction, true);
}


//-----------------------------------------------------------------------------------------------
// Constructs all positions and UVs used by the entire map from the heightmap image
// All positions are defined in world space
//
void Map::ConstructMapVertexList(Image* heightMap)
{
	CalculateInitialPositionsAndUVs(heightMap);

	// Now Calculate all normals and tangents
	for (int currYIndex = 0; currYIndex < m_mapCellLayout.y; ++currYIndex)
	{
		for (int currXIndex = 0; currXIndex < m_mapCellLayout.x; ++ currXIndex)
		{
			IntVector2 currCoords = IntVector2(currXIndex, currYIndex);
			Vector3 currPosition = GetPositionAtVertexCoord(currCoords);

			// Get all neighbor positions first, in a linear list
			std::vector<Vector3> neighborPositions;

			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2(-1,  1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2( 0,  1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2( 1,  1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2( 1,  0))); // Tangent
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2( 1, -1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2( 0, -1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2(-1, -1)));
			neighborPositions.push_back(GetPositionAtVertexCoord(currCoords + IntVector2(-1,  0)));


			// Construct the normals and tangents
			Vector3 finalNormal = Vector3::ZERO;

			for (int positionIndex = 0; positionIndex < (int) neighborPositions.size(); ++positionIndex)
			{
				Vector3 firstPosition = neighborPositions[positionIndex];
				
				int secondIndex = positionIndex + 1;
				if (positionIndex == (int) (neighborPositions.size() - 1))
				{
					secondIndex = 0;
				}

				Vector3 secondPosition = neighborPositions[secondIndex];

				Vector3 a = firstPosition - currPosition;
				Vector3 b = secondPosition - currPosition;

				Vector3 currNormal = CrossProduct(a, b);
				finalNormal += currNormal;
			}

			finalNormal.NormalizeAndGetLength();
			finalNormal *= -1.0f;

			int vertexIndex = currYIndex * m_mapVertexLayout.x + currXIndex;

			m_mapVertices[vertexIndex].normal = finalNormal;

			// Tangent end point is the 5th position processed, so 4th index
			Vector3 tangent = (neighborPositions[3] - currPosition).GetNormalized();
			
			m_mapVertices[vertexIndex].tangent = Vector4(tangent, 1.0f);
		}	
	}
}


//-----------------------------------------------------------------------------------------------
// Calculates the positions and uvs for each vertex in the map
//
void Map::CalculateInitialPositionsAndUVs(Image* image)
{
	m_mapVertices.resize(image->GetTexelCount());

	IntVector2	imageDimensions = image->GetTexelDimensions();
	Vector2		worldDimensions = m_worldBounds.GetDimensions();

	float xStride = worldDimensions.x / (float) (m_mapCellLayout.x);
	float zStride = worldDimensions.y / (float) (m_mapCellLayout.y);

	for (int texelYIndex = 0; texelYIndex < imageDimensions.y; ++texelYIndex)
	{
		for (int texelXIndex = 0; texelXIndex < imageDimensions.x; ++texelXIndex)
		{
			unsigned int mapVertexIndex = texelYIndex * imageDimensions.x + texelXIndex;

			// Determine the UV
			float u = ((float) texelXIndex / ((float) imageDimensions.x - 1));
			float v = 1.0f - ((float) texelYIndex / ((float) imageDimensions.y - 1));

			Vector2 uv = Vector2(u, v);
			m_mapVertices[mapVertexIndex].uv = uv;

				// Determine the position
			float x = m_worldBounds.mins.x + texelXIndex * xStride;
			float z = m_worldBounds.maxs.y - texelYIndex * zStride;
			float y = image->GetTexelGrayScale(texelXIndex, texelYIndex);
			y = RangeMapFloat(y, 0.f, 1.f, m_heightRange.min, m_heightRange.max);

			Vector3 position = Vector3(x, y, z);
			m_mapVertices[mapVertexIndex].position = position;
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
	Sampler* testSampler = new Sampler();

	testSampler->Initialize(SAMPLER_FILTER_LINEAR_MIPMAP_LINEAR, EDGE_SAMPLING_CLAMP_TO_EDGE);
	mapMaterial->SetSampler(0, testSampler);

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


void Map::UpdateEntities()
{
	for (int entityIndex = 0; entityIndex < (int) m_gameEntities.size(); ++entityIndex)
	{
		m_gameEntities[entityIndex]->Update(Game::GetDeltaTime());
	}

	// Update the player
	Game::GetPlayer()->Update(Game::GetDeltaTime());
}

//-----------------------------------------------------------------------------------------------
// Checks for bullet collisions against actors, and marks them for delete if so
//
void Map::CheckProjectilesAgainstActors()
{
	for (int bulletIndex = 0; bulletIndex < (int) m_bullets.size(); ++bulletIndex)
	{
		Bullet* currBullet = m_bullets[bulletIndex];

		for (int tankIndex = 0; tankIndex < (int) m_npcTanks.size(); ++tankIndex)
		{
			NPCTank* currTank = m_npcTanks[tankIndex];

			if (currTank->GetTeamIndex() != currBullet->GetTeamIndex())
			{
				if (DoSpheresOverlap(currBullet->transform.position, currBullet->GetPhysicsRadius(), currTank->transform.position, currTank->GetPhysicsRadius()))
				{
					currBullet->SetMarkedForDelete(true);
					currTank->TakeDamage(100);
				}
			}
		}

		// Also check against the player
		Player* player = Game::GetPlayer();
		if (Game::GetPlayer()->GetTeamIndex() != currBullet->GetTeamIndex())
		{
			if (DoSpheresOverlap(currBullet->transform.position, currBullet->GetPhysicsRadius(), player->transform.position, player->GetPhysicsRadius()))
			{
				currBullet->SetMarkedForDelete(true);
				// Have the player take damage
				Game::GetPlayer()->TakeDamage(currBullet->GetDamageAmount());
			}
		}
	}
}


void Map::CheckActorActorCollisions()
{
}

void Map::UpdateHeightAndOrientationOnMap()
{
	for (int tankIndex = 0; tankIndex < (int) m_npcTanks.size(); ++tankIndex)
	{
		m_npcTanks[tankIndex]->UpdateHeightOnMap();
		m_npcTanks[tankIndex]->UpdateOrientationWithNormal();
	}

	Game::GetPlayer()->UpdateHeightOnMap();
	Game::GetPlayer()->UpdateOrientationWithNormal();
}


//-----------------------------------------------------------------------------------------------
// Checks for entities that are marked for delete, and if so deletes and removes them
//
void Map::DeleteObjectsMarkedForDelete()
{
	// Bullets
	{
	int numBullets = (int) m_bullets.size();
	for (int bulletIndex = numBullets - 1; bulletIndex >= 0; --bulletIndex)
	{
		Bullet* currBullet = m_bullets[bulletIndex];

		if (currBullet->IsMarkedForDelete())
		{
			int numEntities = (int) m_gameEntities.size();
			for (int entityIndex = numEntities - 1; entityIndex >= 0; --entityIndex)
			{
				GameEntity* currEntity = m_gameEntities[entityIndex];

				if (currEntity == currBullet)
				{
					// Remove fast
					m_bullets[bulletIndex] = m_bullets[numBullets - 1]; 
					m_bullets.erase(m_bullets.begin() + numBullets - 1);
					numBullets--;

					m_gameEntities[entityIndex] = m_gameEntities[numEntities - 1];
					m_gameEntities.erase(m_gameEntities.begin() + numEntities - 1);
					numEntities--;

					delete currBullet;
					currBullet = nullptr;
					break;
				}
			}

			if (currBullet == nullptr)
			{
				break;
			}
		}
	}
	}

	{
	// Tanks
	int numTanks = (int) m_npcTanks.size();
	for (int tankIndex = numTanks - 1; tankIndex >= 0; --tankIndex)
	{
		NPCTank* currTank = m_npcTanks[tankIndex];

		if (currTank->IsMarkedForDelete())
		{
			int numEntities = (int) m_gameEntities.size();
			for (int entityIndex = numEntities - 1; entityIndex >= 0; --entityIndex)
			{
				GameEntity* currEntity = m_gameEntities[entityIndex];

				if (currEntity == currTank)
				{
					// Remove fast
					m_npcTanks[tankIndex] = m_npcTanks[numTanks - 1]; 
					m_npcTanks.erase(m_npcTanks.begin() + numTanks - 1);
					numTanks--;

					m_gameEntities[entityIndex] = m_gameEntities[numEntities - 1];
					m_gameEntities.erase(m_gameEntities.begin() + numEntities - 1);
					numEntities--;

					delete currTank;
					currTank = nullptr;
					break;
				}
			}

			if (currTank == nullptr)
			{
				break;
			}
		}
	}
	}

	{
	// Spawners
	int numSpawners = (int) m_spawners.size();
	for (int spawnerIndex = numSpawners - 1; spawnerIndex >= 0; --spawnerIndex)
	{
		NPCSpawner* currSpawner = m_spawners[spawnerIndex];

		if (currSpawner->IsMarkedForDelete())
		{
			int numEntities = (int) m_gameEntities.size();
			for (int entityIndex = numEntities - 1; entityIndex >= 0; --entityIndex)
			{
				GameEntity* currEntity = m_gameEntities[entityIndex];

				if (currEntity == currSpawner)
				{
					// Remove fast
					m_spawners[spawnerIndex] = m_spawners[numSpawners - 1]; 
					m_spawners.erase(m_spawners.begin() + numSpawners - 1);
					numSpawners--;

					m_gameEntities[entityIndex] = m_gameEntities[numEntities - 1];
					m_gameEntities.erase(m_gameEntities.begin() + numEntities - 1);
					numEntities--;

					delete currSpawner;
					currSpawner = nullptr;
					break;
				}
			}

			if (currSpawner == nullptr)
			{
				break;
			}
		}
	}
	}

	// Don't delete the player, just reset them
	Player* player = Game::GetPlayer();
	if (player->IsMarkedForDelete())
	{
		player->SetMarkedForDelete(false);
		player->SetHealth(10);
		player->transform.position = Vector3::ZERO;
		DebugRenderSystem::Draw2DText("Player Died, respawning at (0,0)", Window::GetInstance()->GetWindowBounds(), 3.f, Rgba::RED, 50.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Determines the hit of a Raycast given the positions before and after when the hit occurred on the terrain
//
RaycastHit_t Map::ConvergeRaycastOnTerrain(Vector3& positionBeforeHit, Vector3& positionAfterhit)
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
			return RaycastHit_t(true, mapPosition, false);
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
	return RaycastHit_t(true, midpoint, false);
}


//-----------------------------------------------------------------------------------------------
// Determines the hit of a Raycast given the positions before and after when the hit occurred on an object
//
RaycastHit_t Map::ConvergeRaycastOnObject(Vector3& positionBeforeHit, Vector3& positionAfterHit, const GameObject* object)
{
	Vector3 midpoint;
	Vector3 objectPosition = object->transform.position;
	float radiusSquared = object->GetPhysicsRadius();
	radiusSquared *= radiusSquared;

	for (int iteration = 0; iteration < RAYCAST_CONVERGE_ITERATION_COUNT; ++iteration)
	{
		midpoint = (positionAfterHit + positionBeforeHit) * 0.5f;

		// Check for early out
		float distanceSquared = (objectPosition - midpoint).GetLengthSquared();
		float midDelta = distanceSquared - radiusSquared;

		if (AbsoluteValue(midDelta) < RAYCAST_CONVERGE_EARLYOUT_DISTANCE)
		{
			return RaycastHit_t(true, midpoint, false);
		}

		// No early out, so update the endpositions and continue iterating
		if (midDelta > 0.f)
		{
			// Still outside radius, so update start
			positionBeforeHit = midpoint;
		}
		else
		{
			// Midpoint is inside radius, so update end
			positionAfterHit = midpoint;
		}
	}

	// Didn't fully converged, so just return our last midpoint
	return RaycastHit_t(true, midpoint, false);
}
