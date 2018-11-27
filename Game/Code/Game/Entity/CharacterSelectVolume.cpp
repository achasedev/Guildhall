#include "Game/Entity/Player.hpp"
#include "Game/Entity/CharacterSelectVolume.hpp"


CharacterSelectVolume::CharacterSelectVolume(const EntityDefinition* definition)
	: Entity(definition)
{
}

void CharacterSelectVolume::OnEntityCollision(Entity* other)
{
	if (other->IsPlayer())
	{
		Player* player = dynamic_cast<Player*>(other);

		if (player->GetEntityDefinition() != m_definition->m_playerCharacterDefinition)
		{
			player->ReinitializeWithDefinition(m_definition->m_playerCharacterDefinition);
		}
	}
}
