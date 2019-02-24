/************************************************************************/
/* File: BehaviorComponent.cpp
/* Author: Andrew Chase
/* Date: October 15th 2018
/* Description: Implementation of the BehaviorComponent base class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/AIEntity.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Entity/Components/BehaviorComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
BehaviorComponent::BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
BehaviorComponent::~BehaviorComponent()
{
}


//-----------------------------------------------------------------------------------------------
// Initialize
//
void BehaviorComponent::Initialize(AIEntity* owningEntity)
{
	m_owningEntity = owningEntity;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void BehaviorComponent::Update()
{
	m_closestPlayer = GetClosestAlivePlayer();
}


//-----------------------------------------------------------------------------------------------
// For when we need to set parameters or initialize after the entity has spawned
//
void BehaviorComponent::OnSpawn()
{
	m_closestPlayer = GetClosestAlivePlayer();
}


//-----------------------------------------------------------------------------------------------
// Callback for handling entity behavior on collisions
//
void BehaviorComponent::OnEntityCollision(Entity* other)
{
	UNUSED(other);
}


//-----------------------------------------------------------------------------------------------
// Returns the closest player to the entity with this component
//
Player* BehaviorComponent::GetClosestAlivePlayer() const
{
	Player** players = Game::GetPlayers();

	Vector3 closestPlayerPosition;
	float minDistance = 10000.f;
	Player* closestPlayer = nullptr;

	Vector3 currentPosition = m_owningEntity->GetPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr && !players[i]->IsRespawning())
		{
			Vector3 playerPosition = players[i]->GetPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			if (closestPlayer == nullptr || currDistance < minDistance)
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				closestPlayer = players[i];
			}
		}
	}

	return closestPlayer;
}


//-----------------------------------------------------------------------------------------------
// Returns the closest player the entity has line of sight on
//
Player* BehaviorComponent::GetClosestPlayerInSight() const
{
	return Game::GetPlayers()[0];
}


//-----------------------------------------------------------------------------------------------
// Returns the distance this component's entity is from the closest player
//
float BehaviorComponent::GetDistanceToClosestPlayer() const
{
	if (m_closestPlayer == nullptr)
	{
		return 999999999.f;
	}

	Vector2 playerPos = m_closestPlayer->GetPosition().xz();
	Vector2 currPos = m_owningEntity->GetPosition().xz();

	return (currPos - playerPos).GetLength();
}


//-----------------------------------------------------------------------------------------------
// Moves the entity towards the closest player
//
void BehaviorComponent::MoveToClosestPlayer()
{
	Vector3 closestPlayerPosition = m_closestPlayer->GetPosition();
	Vector3 currPosition = m_owningEntity->GetPosition();

	Vector3 directionToPlayer = (closestPlayerPosition - currPosition).GetNormalized();

	m_owningEntity->Move(directionToPlayer.xz());
	m_owningEntity->Decelerate();
}


//-----------------------------------------------------------------------------------------------
// For avoiding obstacles, will return the vector direction to weight into the primary move direction
// If no obstacle, returns (0,0)
//
Vector2 BehaviorComponent::GetDirectionToAvoidClosestStaticObstacle(const Vector2& targetDirection) const
{
	std::vector<Entity*> entities = Game::GetWorld()->GetEntitiesThatOverlapSphere(m_owningEntity->GetCenterPosition(), 10.f);
	Vector2 directionToClosestStaticObstacle = Vector2::ZERO;
	Vector2 entityForward = m_owningEntity->GetForwardVector().xz();
	float distanceToClosestStatic = 10000.f;

	// Check all nearby entities
	for (int entityIndex = 0; entityIndex < (int)entities.size(); ++entityIndex)
	{
		Entity* entity = entities[entityIndex];

		if (entity == m_owningEntity) // Don't compare to ourselves
		{
			continue;
		}
		else if (entity->GetPhysicsType() == PHYSICS_TYPE_STATIC) // Walk around a static
		{
			Vector2 directionToObstacle = entity->GetCenterPosition().xz() - m_owningEntity->GetCenterPosition().xz();
			float currDistance = directionToObstacle.NormalizeAndGetLength();

			bool withinCone = DotProduct(directionToObstacle, entityForward) > CosDegrees(45.f);

			if (withinCone && currDistance < distanceToClosestStatic)
			{
				distanceToClosestStatic = currDistance;
				directionToClosestStaticObstacle = directionToObstacle;
			}
		}
	}

	Vector2 directionToAvoidObstacle = Vector2::ZERO;
	if (directionToClosestStaticObstacle != Vector2::ZERO)
	{
		float sign = (GetAngularDisplacement(targetDirection.GetOrientationDegrees(), directionToClosestStaticObstacle.GetOrientationDegrees()) < 0.f ? 1.0f : -1.0f);
		directionToAvoidObstacle = sign * Vector2(-directionToClosestStaticObstacle.y, directionToClosestStaticObstacle.x);
	}

	return directionToAvoidObstacle;
}
