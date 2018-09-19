#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Game/Entity/Entity.hpp"

class DynamicEntity : public Entity
{
public:
	//-----Public Methods-----

	DynamicEntity();
	~DynamicEntity();

	virtual void Update() override;

	void ApplyForce(const Vector3& force);

	void ApplyPhysicsStep();

private:
	//-----Private Methods-----



private:
	//-----Private Data-----

	Vector3 m_velocity			= Vector3::ZERO;
	Vector3 m_acceleration		= Vector3::ZERO;
	Vector3 m_force				= Vector3::ZERO;
	float	m_mass				= 1.f;
	float	m_inverseMass		= 1.f;
	bool	m_affectedByGravity	= true;

};
