/************************************************************************/
/* File: CharacterSelectVolume.cpp
/* Author: Andrew Chase
/* Date: November 24th 2018
/* Description: Implementation of the CharacterSelectVolume class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Entity/CharacterSelectVolume.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
CharacterSelectVolume::CharacterSelectVolume(const EntityDefinition* definition)
	: Entity(definition)
{
}


//-----------------------------------------------------------------------------------------------
// Collision callback
//
void CharacterSelectVolume::OnEntityCollision(Entity* other)
{
	if (m_playerThatChoseMe == nullptr && other->IsPlayer())
	{
		Player* player = dynamic_cast<Player*>(other);

		if (player->GetEntityDefinition() != m_definition->m_playerCharacterDefinition)
		{
			player->ReinitializeWithDefinition(m_definition->m_playerCharacterDefinition);
			m_playerThatChoseMe = player;

			m_shouldRender = false;
		}

		Game::GetWorld()->ParticalizeEntity(this);
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void CharacterSelectVolume::Update()
{
	// Check if the player that chose me is still me and not something else
	if (m_playerThatChoseMe != nullptr)
	{
		const EntityDefinition* playersChoice = m_playerThatChoseMe->GetEntityDefinition();

		if (playersChoice != m_definition->m_playerCharacterDefinition)
		{
			m_playerThatChoseMe = nullptr;
			m_shouldRender = true;
		}
	}
}
