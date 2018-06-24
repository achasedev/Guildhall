/************************************************************************/
/* File: NPCTank.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the NPCTank class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/Cannon.hpp"
#include "Game/Entity/Turret.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/NPCTank.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Constants
const float NPCTank::MIN_DOT_TO_MOVE_FORWARD = 0.707f; // Cos(45 degrees)
const float NPCTank::MIN_DOT_TO_SHOOT = 0.96f; // Cos(15 degrees)


//-----------------------------------------------------------------------------------------------
// Constructor
//
NPCTank::NPCTank(unsigned int team)
	: Tank(team)
{
	m_health = 1;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void NPCTank::Update(float deltaTime)
{
	// Set the target to be the player's center position for now
	SetTarget(true, Game::GetPlayer()->transform.position + Vector3(0.f, 0.5f, 0.f));
	
	// Moving
	if (m_hasTarget)
	{
		RotateTowardsTarget();

		if (IsAlmostFacingTarget())
		{
			MoveForward();
		}

		// Shooting
		if (CanShootAtTarget())
		{
			ShootCannon();
		}
	}

	Tank::Update(deltaTime);	// Will update the height on the map for us
}


//-----------------------------------------------------------------------------------------------
// Rotates the tank towards the target position
//
void NPCTank::RotateTowardsTarget()
{
	// Get transforms
	Matrix44 toWorld = transform.GetWorldMatrix();
	Matrix44 toLocal = toWorld.GetInverse();

	// Convert to local space, snap to xz-plane
	Vector3 localPosition = toLocal.TransformPoint(m_target).xyz();
	localPosition.y = 0.f;

	// Get start and end angle values
	Vector3 currentRotation = transform.rotation.GetAsEulerAngles();
	Vector2 dirToTarget = (m_target - transform.position).xz();

	float startAngle = currentRotation.y;
	float endAngle = 180.f - (dirToTarget.GetOrientationDegrees() + 90.f);

	// Find new Y rotation
	float deltaTime = Game::GetDeltaTime();
	float newAngle = TurnToward(startAngle, endAngle, TANK_ROTATION_SPEED * deltaTime);

	// Rotate
	currentRotation.y = newAngle;
	transform.rotation = Quaternion::FromEuler(currentRotation);

	// Also turn the turret and cannon to point at the target
	m_turret->TurnTowardsTarget(m_target);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the turret body is within the minimum threshold for facing the target
//
bool NPCTank::IsAlmostFacingTarget()
{
	Vector2 dirToTarget = (m_target - transform.GetWorldPosition()).xz();
	dirToTarget.NormalizeAndGetLength();

	Vector2 forward = transform.GetWorldForward().xz();
	forward.NormalizeAndGetLength();

	float dot = DotProduct(dirToTarget, forward);

	return dot >= MIN_DOT_TO_MOVE_FORWARD;
}


//-----------------------------------------------------------------------------------------------
// Translates the target along its forward vector
//
void NPCTank::MoveForward()
{
	float deltaTime = Game::GetDeltaTime();
	Vector3 worldForward = transform.GetWorldForward();
	worldForward.y = 0;
	worldForward.NormalizeAndGetLength();

	Vector3 forwardTranslation = worldForward * (TANK_TRANSLATION_SPEED * deltaTime);

	transform.TranslateWorld(forwardTranslation);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the tank is mostly aiming at the target, and has line of sight
//
bool NPCTank::CanShootAtTarget()
{
	Map* map = Game::GetMap();

	Cannon* cannon = m_turret->GetCannon();
	Vector3 cannonDirection = cannon->transform.GetWorldForward();
	cannonDirection.GetNormalized();

	Vector3 cannonWorldPosition = cannon->transform.GetWorldPosition();

	Vector3 dirToTarget = (m_target - cannonWorldPosition);
	float distance = dirToTarget.NormalizeAndGetLength();

	float dot = DotProduct(dirToTarget, cannonDirection);

	// If the cannon is mostly pointing at the target...
	if (dot >= MIN_DOT_TO_SHOOT)
	{
		RaycastHit_t raycastHit = map->Raycast(cannonWorldPosition, dirToTarget, distance);

		// If we managed to reach the target, then we have line of sight, so return true
		return raycastHit.wentFullDistance;
	}

	return false;
}
