#pragma once
#include "Engine/Core/GameObject.hpp"

class GameEntity : public GameObject
{
public:
	//-----Public Methods-----

	virtual void Update(float deltaTime) override;

	inline unsigned int GetTeamIndex() { return m_team; }


protected:
	//-----Protected Data-----

	unsigned int m_team;
};