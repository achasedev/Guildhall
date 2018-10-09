/************************************************************************/
/* File: BehaviorComponent_Wander.cpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Implementation of the wander behavior
/************************************************************************/
#include "Game/Entity/MovingEntity.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Components/BehaviorComponent_Wander.hpp"
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

BehaviorComponent_Wander::BehaviorComponent_Wander(float wanderInterval)
	: m_wanderInterval(wanderInterval)
{
	m_timer.SetClock(Game::GetGameClock());
	m_timer.Reset();
}

BehaviorComponent_Wander::~BehaviorComponent_Wander()
{
}

void BehaviorComponent_Wander::Update()
{ 
	if (Game::GetDeltaTime() > 0.4f)
	{
		return;
	}

	BehaviorComponent::Update();

	// Check to get a new path
	if (m_timer.HasIntervalElapsed())
	{
		// Get a new position
		m_finalPosition = Vector3(GetRandomFloatInRange(20.f, 240.f), 4.f, GetRandomFloatInRange(20.f, 240.f));

		// Update our distance map
		m_navigationMap->Clear(999.f);
		IntVector2 targetCoords = IntVector2(m_finalPosition.xz());
		m_finalPosition = Vector3((float)targetCoords.x + 0.5f, 4.f, (float)targetCoords.y + 0.5f);

		DebugRenderSystem::DrawUVSphere(m_finalPosition, m_wanderInterval, Rgba::RED, 5.f);

		m_navigationMap->SetHeat(targetCoords, 0.f);

		m_navigationMap->SolveMapUpToDistance(999.f); // Will need to provide a cost map here from world

		std::vector<IntVector2> path;

		IntVector2 currCoords = m_owningEntity->GetEntityCoordinatePosition().xz();
		m_navigationMap->GetGreedyShortestPath(currCoords, targetCoords, path);

		for (int i = 0; i < (int) path.size(); ++i)
		{
			IntVector2 currPathCoords = path[i];
			Vector3 currPathPos = Vector3((float) currPathCoords.x + 0.5f, 4.f, (float) currPathCoords.y + 0.5f);

			DebugRenderSystem::DrawUVSphere(currPathPos, 5000.f, Rgba::CYAN, 1.0f);
		}

		IntVector2 nextCoords = m_navigationMap->GetMinNeighborCoords(currCoords);

		m_nextPosition = Vector3((float) nextCoords.x + 0.5f, 4.f, (float) nextCoords.y + 0.5f);
		//DebugRenderSystem::DrawUVSphere(m_nextPosition, 5.f, Rgba::YELLOW, 1.f);

		m_timer.SetInterval(m_wanderInterval);
	}

	// Move towards next position
	Vector2 toNext = (m_nextPosition - m_owningEntity->GetEntityPosition()).xz();
	m_owningEntity->SetOrientation(toNext.GetOrientationDegrees());

	float distance = toNext.NormalizeAndGetLength();

	// Check to update the next position
	if (distance < 5.f)
	{
		IntVector2 currCoords = m_owningEntity->GetEntityCoordinatePosition().xz();
		IntVector2 nextCoords = m_navigationMap->GetMinNeighborCoords(currCoords);
		m_nextPosition = Vector3((float)nextCoords.x + 0.5f, 4.f, (float)nextCoords.y + 0.5f);

		//DebugRenderSystem::DrawUVSphere(m_nextPosition, 5.f, Rgba::YELLOW, 1.f);

		toNext = (m_nextPosition - m_owningEntity->GetEntityPosition()).xz();
		m_owningEntity->Move(toNext.GetNormalized());
	}
	else
	{
		m_owningEntity->Move(toNext);
	}
}

BehaviorComponent* BehaviorComponent_Wander::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr && m_navigationMap == nullptr, "Error: Behavior clone had non-null base members on prototype");
	BehaviorComponent* clone = new BehaviorComponent_Wander(m_wanderInterval);

	return clone;
}
