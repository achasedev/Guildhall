/************************************************************************/
/* File: PlayMode.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Base Class for input-based events during play
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector3.hpp"

class Actor;
class ActorController;
class Rgba;

class PlayMode
{
public:
	//-----Public Methods-----

	PlayMode(ActorController* controller);

	// Virtual functions
	virtual void Start();
	virtual void ProcessInput();
	virtual void Update();
	virtual void RenderWorldSpace() const;
	virtual void RenderScreenSpace() const;
	virtual void Exit();

	virtual bool IsStarted() const;
	virtual bool IsFinished() const;


protected:
	//-----Protected Methods-----

	// Render - for the cursor and other tiles used by derived classes
	void RenderSingleTile(const IntVector3& tileCoords, const Rgba& color) const;


private:
	//-----Private Methods-----

	// Input - updates the cursor
	void ProcessCursorInput();
	
	void RenderHoverData() const;
	void RenderTurnOrderWidget() const;


protected:
	//-----Protected Data-----

	ActorController*	m_controller;			// Controller of the current actor
	IntVector3			m_cursorCoords;			// The cursor
	Actor*				m_hoverActor;			// The actor the cursor is currently on
	bool				m_isFinished = false;	// Is this mode done?
	bool				m_isStarted = false;	// Has this mode begun updating?

	static AABB2		s_hoverBounds;
	static AABB2		s_turnOrderBounds;
};
