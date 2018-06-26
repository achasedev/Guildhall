/************************************************************************/
/* File: Cannon.cpp
/* Author: Andrew Chase
/* Date: June 18th, 2018
/* Description: Implementation of the Cannon class
/************************************************************************/
#include "Game/Entity/Cannon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Math/Matrix44.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// Constants
const float Cannon::CANNON_ROTATION_SPEED = 30.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
Cannon::Cannon(Transform& parent)
	: m_angleLimits(FloatRange(-45.f, 10.f))
{
	transform.SetParentTransform(&parent);

	// Set up the cannon renderable
	m_renderable = new Renderable();
	RenderableDraw_t draw;
	draw.sharedMaterial = AssetDB::GetSharedMaterial("Data/Materials/Tank.material");
	draw.mesh = AssetDB::GetMesh("Cube");
	draw.drawMatrix = Matrix44::MakeModelMatrix(Vector3(0.f, 0.25f, 1.5f), Vector3::ZERO, Vector3(0.2f, 0.2f, 2.f));

	m_renderable->AddDraw(draw);
	m_renderable->AddInstanceMatrix(transform.GetWorldMatrix());

	Game::GetRenderScene()->AddRenderable(m_renderable);

	// Set up the muzzle transform
	m_muzzleTransform.SetParentTransform(&transform);
	m_muzzleTransform.position = Vector3(0.f, 0.f, 3.f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
Cannon::~Cannon()
{
	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
	m_renderable = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Update
//
void Cannon::Update(float deltaTime)
{
	UNUSED(deltaTime);
	m_renderable->SetInstanceMatrix(0, transform.GetWorldMatrix());

	// Also show where the gun is aiming
	Vector3 position = m_muzzleTransform.GetParentsToWorldMatrix().TransformPoint(m_muzzleTransform.position).xyz();
	Vector3 direction = m_muzzleTransform.GetWorldForward();
	RaycastHit_t hit = Game::GetMap()->Raycast(position, direction, Map::MAX_RAYCAST_DISTANCE);

	// Don't actually care if it hit, just render a debug to the end position
	DebugRenderSystem::Draw3DLine(position, hit.position, Rgba::RED, 0.f);
}


//-----------------------------------------------------------------------------------------------
// Returns the world muzzle transform, for spawning projectiles
//
Matrix44 Cannon::GetFireTransform()
{
	return m_muzzleTransform.GetWorldMatrix();
}


//-----------------------------------------------------------------------------------------------
// Aims the cannon up/down towards the given target
//
void Cannon::ElevateTowardsTarget(const Vector3& target)
{
	// Get the transforms
	Matrix44 toParent = transform.GetLocalMatrix();
	Matrix44 toLocal = transform.GetWorldMatrix().GetInverse();

	// Convert to local space, and snap to the xy plane
	Vector3 localPosition = toLocal.TransformPoint(target).xyz();
	localPosition.x = 0.f;

	// Convert back to parents space
	Vector3 parentPosition = toParent.TransformPoint(localPosition).xyz();

	// Find the start and end angles for the movement
	Vector3 currentRotation = transform.rotation.GetAsEulerAngles();

	Vector3 dirToTarget3D = (parentPosition - transform.position);
	Vector2 dirToTarget = Vector2(dirToTarget3D.z, dirToTarget3D.y);

	float startAngle = currentRotation.x;
	float endAngle = -dirToTarget.GetOrientationDegrees();

	// Determine the final x rotation
	float deltaTime = Game::GetDeltaTime();
	float newAngle = TurnToward(startAngle, endAngle, CANNON_ROTATION_SPEED * deltaTime);

	// Clamp the angle to avoid the gun clipping or aiming too high
	newAngle = ClampFloat(newAngle, m_angleLimits.min, m_angleLimits.max);

	// Set the rotation
	currentRotation.x = newAngle;
	transform.rotation = Quaternion::FromEuler(currentRotation);
}
