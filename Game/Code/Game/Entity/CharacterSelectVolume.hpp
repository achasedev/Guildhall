/************************************************************************/
/* File: CharacterSelectVolume.hpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Class to represent a trigger volume for the character select
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

class CharacterSelectVolume : public Entity
{
public:
	//-----Public Methods-----
	
	CharacterSelectVolume(const EntityDefinition* definition);
	virtual void OnEntityCollision(Entity* other) override;
	virtual void Update() override;

	
private:
	//-----Private Data-----
	
	Player* m_playerThatChoseMe = nullptr;

};
