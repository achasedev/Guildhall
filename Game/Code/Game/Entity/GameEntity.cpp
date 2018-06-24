#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/GameEntity.hpp"
#include "Engine/Math/MathUtils.hpp"

GameEntity::GameEntity(eEntityType type)
	: m_type(type)
{
}

void GameEntity::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}

void GameEntity::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;

	if (m_health <= 0)
	{
		SetMarkedForDelete(true);
	}
}

void GameEntity::SetHealth(int health)
{
	m_health = health;
}


//-----------------------------------------------------------------------------------------------
// Sets the tanks position to be at the appropriate height of the terrain
//
void GameEntity::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(transform.position);
	transform.position.y = height;
}


//-----------------------------------------------------------------------------------------------
// Orients the tank so that its up vector lines up with the normal at the current position
//
void GameEntity::UpdateOrientationWithNormal()
{
	Map* map = Game::GetMap();

	Vector3 normal = map->GetNormalAtPosition(transform.position);
	Vector3 right = CrossProduct(normal, transform.GetWorldForward());
	right.NormalizeAndGetLength();

	Vector3 newForward = CrossProduct(right, normal);
	newForward.NormalizeAndGetLength();

	Matrix44 newModel = Matrix44(right, normal, newForward, transform.position);
	transform.SetModelMatrix(newModel);
}
