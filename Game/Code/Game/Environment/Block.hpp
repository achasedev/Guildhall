#pragma once
#include <string>
#include "Engine/Math/AABB3.hpp"

class BlockDefinition;

class Block
{
public:
	//-----Public Methods-----

	Block();
	~Block();

	void SetWorldBounds(const Vector3& mins, const Vector3& maxs);
	void SetDefinition(const std::string& definitionName);
	void SetDefinition(const BlockDefinition* definition);


private:
	//-----Private Data-----

	AABB3					m_worldBounds;
	const BlockDefinition*	m_definition = nullptr;

};
