#include "Game/Behavior_Flee.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/XmlUtilities.hpp"

Behavior_Flee::Behavior_Flee(const XMLElement& behaviorElement)
	: Behavior(behaviorElement)
	, m_fleeTimer(TIME_PER_FLEE)
{
	m_minSquaredDistanceFromPlayer = ParseXmlAttribute(behaviorElement, "minFleeDistance", m_minSquaredDistanceFromPlayer);
}

void Behavior_Flee::Update(float deltaTime)
{
	// Update the flee position at determined intervals
	UpdateFleeTargetPosition();

	float squaredDistanceToTarget = GetDistanceSquared(m_fleePosition, m_actor->GetPosition());

	// If we reached our target stop moving
	if (squaredDistanceToTarget < 0.25f)
	{
		m_actor->SetVelocity(Vector2::ZERO);
	}
	else
	{
		// Use a distance map for path finding
		IntVector2 targetCoords = IntVector2(static_cast<int>(m_fleePosition.x), static_cast<int>(m_fleePosition.y));

		// Set the velocity to move the actor along the path
		Vector2 currPosition = m_actor->GetPosition();
		IntVector2 currCoords = IntVector2(static_cast<int>(currPosition.x), static_cast<int>(currPosition.y));
		SetActorVelocityAlongPath(targetCoords);
	}

	m_fleeTimer -= deltaTime;
}


float Behavior_Flee::CalcUtility()
{
	float currentHealth = static_cast<float>(m_actor->GetCurrentHealth());
	float maxHealth = static_cast<float>(m_actor->GetMaxHealth());

	return 1.f - (currentHealth / maxHealth);
}


Behavior* Behavior_Flee::Clone() const
{
	return new Behavior_Flee(*this);
}


void Behavior_Flee::UpdateFleeTargetPosition()
{
	if (m_fleeTimer < 0.f || m_fleePosition == Vector2::ZERO)
	{
		m_fleeTimer = TIME_PER_FLEE;

		bool foundNewTargetPosition = false;

		while (!foundNewTargetPosition)
		{
			Tile* newTargetTile		= m_actor->GetMap()->GetTileEntityCanEnter(m_actor);
			IntVector2 targetCoords = newTargetTile->GetTileCoords();	

			if (AreTargetCoordsAccessible(targetCoords))
			{
				foundNewTargetPosition = true;
				m_fleePosition = newTargetTile->GetCenterPosition();
			}	
		}
	}
}

