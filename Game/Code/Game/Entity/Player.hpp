#pragma once
#include "Game/Entity/DynamicEntity.hpp"

class VoxelGrid;

class Player : public DynamicEntity
{
public:


	void ProcessInput();
	virtual void Update() override;


private:

	int m_playerID = 0;
};