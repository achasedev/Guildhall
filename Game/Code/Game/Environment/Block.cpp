#include "Game/Environment/Block.hpp"
#include "Game/Environment/BlockDefinition.hpp"

#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"

#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

Block::Block()
{
}


Block::~Block()
{

}

void Block::SetWorldBounds(const Vector3& mins, const Vector3& maxs)
{
	m_worldBounds.mins = mins;
	m_worldBounds.maxs = maxs;
}

void Block::SetDefinition(const std::string& definitionName)
{
	const BlockDefinition* definition = BlockDefinition::GetDefinitionByName(definitionName);
	SetDefinition(definition);
}

void Block::SetDefinition(const BlockDefinition* definition)
{
	m_definition = definition;
}
