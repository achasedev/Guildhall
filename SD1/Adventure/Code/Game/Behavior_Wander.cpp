#include "Game/Behavior_Wander.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/XmlUtilities.hpp"


Behavior_Wander::Behavior_Wander(const XMLElement& behaviorElement)
	: Behavior(behaviorElement)
	, m_wanderTimer(0.f)
{
	m_maxDistancePerMove = ParseXmlAttribute(behaviorElement, "maxDistancePerMove", m_maxDistancePerMove);
}

void Behavior_Wander::Update(float deltaTime)
{
	// Update the wander target if the timer is up
	UpdateWanderTarget();

	// Get relevant positions
	Vector2 actorPos = m_actor->GetPosition();
	IntVector2 currCoords = IntVector2(actorPos.x, actorPos.y);
	IntVector2 targetCoords = IntVector2(m_wanderPosition.x, m_wanderPosition.y);

	// Move along the path
	SetActorVelocityAlongPath(currCoords);

	// Update wander timer
	m_wanderTimer -= deltaTime;
}

float Behavior_Wander::CalcUtility()
{
	return 0.3f;
}

Behavior* Behavior_Wander::Clone() const
{
	return new Behavior_Wander(*this);
}

void Behavior_Wander::UpdateWanderTarget()
{
	if (m_wanderTimer < 0.f || m_wanderPosition == Vector2::ZERO)
	{
		m_wanderTimer = WANDER_TIME;

		m_wanderPosition = GetLocalWanderPosition();	
	}
}

Vector2 Behavior_Wander::GetLocalWanderPosition() const
{
	Map* actorMap = m_actor->GetMap();
	Vector2 actorPos = m_actor->GetPosition();
	IntVector2 currCoords = IntVector2(actorPos.x, actorPos.y);

	// Attempt to find a random position in 10 tries
	for (int i = 0; i < 10; i++)
	{
		IntVector2 randomOffset = IntVector2::GetRandomVector(m_maxDistancePerMove);
		IntVector2 randomTargetCoords = currCoords + randomOffset;

		if (actorMap->AreCoordsValid(randomTargetCoords)) 
		{
			if (AreTargetCoordsAccessible(randomTargetCoords))
			{
				return (Vector2(randomTargetCoords) + Vector2(0.5f, 0.5f));
			}
		}
	}

	// If after 10 attempts an accessible position wasn't found, just return a position within the tile
	return Vector2(currCoords) + Vector2::GetRandomVector(0.1f);
}

