/************************************************************************/
/* File: PlayAction_Move.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Action used for moving an actor on the map
/************************************************************************/
#pragma once
#include <stack>
#include <vector>
#include "Game/PlayAction.hpp"
#include "Engine/Math/IntVector3.hpp"

class HeatMap;

class PlayAction_Move : public PlayAction
{
public:
	//-----Public Methods-----

	PlayAction_Move(ActorController* actorController);

	// Virtual Methods
	virtual void Setup() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;

	// Callback for when a destination tile has been selected
	void OnTileSelection(bool wasCancelled, const IntVector3& selection);


private:
	//-----Private Methods-----

	void ConstructMovePath();
		IntVector3 GetMinNeighbor(const IntVector3& currCoords) const;


private:
	//-----Private Data-----

	IntVector3				m_currCoords;
	IntVector3				m_targetCoords;
	std::stack<IntVector3>	m_movePath;

	HeatMap*				m_distanceMap = nullptr;

	ActorController*		m_controller;
};
