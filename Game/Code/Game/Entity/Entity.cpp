#include "Game/Entity/Entity.hpp"
#include "Game/Framework/Game.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

#include <stdlib.h>

#define MAX_ENTITY_VOXEL_COUNT 8*8*8

Entity::Entity()
{
	m_textures[0] = AssetDB::CreateOrGet3DVoxelTextureInstance("Data/3DTextures/TestCube.qef");

	// Rotate to get the other 3
	IntVector3 dimensions = m_textures[0]->GetDimensions();

	// South
	int destIndex = 0;
	m_textures[DIRECTION_SOUTH] = m_textures[0]->Copy();
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int z = dimensions.z - 1; z >= 0; --z)
		{
			for (int x = dimensions.x - 1; x >= 0; --x)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_SOUTH]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));
				destIndex++;
			}
		}
	}

	// East
	m_textures[DIRECTION_EAST] = m_textures[0]->Copy();
	destIndex = 0;
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int x = dimensions.x - 1; x >= 0; --x)
		{
			for (int z = 0; z < dimensions.z; ++z)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_EAST]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));

				destIndex++;
			}
		}
	}

	// West
	m_textures[DIRECTION_WEST] = m_textures[0]->Copy();
	destIndex = 0;
	for (int y = 0; y < dimensions.y; ++y)
	{
		for (int x = 0; x < dimensions.x; ++x)
		{
			for (int z = dimensions.z - 1; z >= 0; --z)
			{
				int sourceIndex = y * (dimensions.x * dimensions.z) + z * dimensions.x + x;
				m_textures[DIRECTION_WEST]->SetColorAtIndex(destIndex, m_textures[0]->GetColorAtIndex(sourceIndex));

				destIndex++;
			}
		}
	}

	m_position = Vector3(128.f, 0.f, 128.f);
	m_orientation = 0;
	DebugRenderSystem::DrawBasis(Vector3(20.f, 0.f, 20.f), Vector3::ZERO, 10000.f, 16.f);
}

Entity::~Entity()
{
	for (int i = 0; i < NUM_DIRECTIONS; ++i)
	{
		if (m_textures[i] != nullptr)
		{
			delete m_textures[i];
			m_textures[i] = nullptr;
		}
	}
}

void Entity::Update()
{
}

Vector3 Entity::GetPosition() const
{
	return m_position;
}

Texture3D* Entity::GetTextureForOrientation() const
{
	float cardinalAngle = GetNearestCardinalAngle(m_orientation);

	if		(cardinalAngle == 0.f) { return m_textures[DIRECTION_EAST]; }
	else if (cardinalAngle == 90.f) { return m_textures[DIRECTION_NORTH]; }
	else if (cardinalAngle == 180.f) { return m_textures[DIRECTION_WEST]; }
	else	{ return m_textures[DIRECTION_SOUTH]; }
}
