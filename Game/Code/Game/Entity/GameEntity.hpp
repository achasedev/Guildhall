#pragma once
#include "Engine/Core/GameObject.hpp"

enum eEntityType
{
	ENTITY_BULLET,
	ENTITY_TANK,
	ENTITY_SPAWNER,
	ENTITY_SWARMER,
	ENTITY_PLAYER,
	NUM_ENTITY_TYPES
};

class GameEntity : public GameObject
{
public:
	//-----Public Methods-----

	GameEntity(eEntityType type);

	virtual void Update(float deltaTime) override;

	virtual void OnCollisionWithEntity(GameEntity* other);

	inline unsigned int GetTeamIndex() { return m_team; }
	inline eEntityType	GetType() { return m_type; }

	void TakeDamage(int damageAmount);
	void SetHealth(int health);

	inline void		SetPhysicsRadius(float radius) { m_physicsRadius = radius; }
	inline float	GetPhysicsRadius() const { return m_physicsRadius; }

	inline bool ShouldStickToTerrain() const { return m_shouldStickToTerrain; }
	inline bool ShouldOrientToTerrain() const { return m_shouldOrientToTerrain; }

	void UpdateHeightOnMap();
	void UpdateOrientationWithNormal();


protected:
	//-----Protected Data-----

	unsigned int m_team;
	float m_physicsRadius;

	int m_health;

	bool m_shouldStickToTerrain = false;
	bool m_shouldOrientToTerrain = false;

	eEntityType m_type;
};