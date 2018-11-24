#include "Game/Entity/Player.hpp"
#include "Game/Entity/CharacterSelectVolume.hpp"


CharacterSelectVolume::CharacterSelectVolume()
	: Entity(EntityDefinition::GetDefinition("CharacterSelect"))
{
	m_position = Vector3(10.f, 5.f, 10.f);
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
