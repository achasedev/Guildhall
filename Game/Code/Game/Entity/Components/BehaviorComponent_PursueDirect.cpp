#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Components/BehaviorComponent_PursueDirect.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

void BehaviorComponent_PursueDirect::Update()
{
	BehaviorComponent::Update();

	Player** players = Game::GetPlayers();

	Vector3 closestPlayerPosition;
	float minDistance;
	bool playerFound = false;

	Vector3 currentPosition = m_owningEntity->GetEntityPosition();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			Vector3 playerPosition = players[i]->GetEntityPosition();
			float currDistance = (playerPosition - currentPosition).GetLengthSquared();

			if (!playerFound || currDistance < minDistance)
			{
				minDistance = currDistance;
				closestPlayerPosition = playerPosition;
				playerFound = true;
			}
		}
	}
	
	// Shouldn't happen, but to avoid unidentifiable behavior
	if (!playerFound)
	{
		return;
	}

	Vector3 directionToMove = (closestPlayerPosition - currentPosition).GetNormalized();
	m_owningEntity->Move(directionToMove.xz());
}

BehaviorComponent* BehaviorComponent_PursueDirect::Clone() const
{
	ASSERT_OR_DIE(m_owningEntity == nullptr, "Error: Behavior clone had non-null base members on prototype");
	return new BehaviorComponent_PursueDirect();
}
