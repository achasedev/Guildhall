/************************************************************************/
/* File: PlayAction.hpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Base Class for autonomous, uninterrupted events during game play
/************************************************************************/
#pragma once
#include <string>

class Actor;
class ActorController;

// What state an action could be in
enum eActionState
{
	ACTION_STATE_NOT_READY,
	ACTION_STATE_READY,
	ACTION_STATE_RUNNING,
	ACTION_STATE_FINISHED,
	ACTION_STATE_CANCELLED,
	NUM_ACTION_STATES
};

// For determining which moves an actor can use in a turn
// (one move, one attack per turn)
enum eActionClass
{
	ACTION_CLASS_NONE = -1,
	ACTION_CLASS_ATTACK,
	ACTION_CLASS_MOVE,
	ACTION_CLASS_ABILITY,
	ACTION_CLASS_WAIT,
	NUM_ACTION_CLASSES
};

// For determining which action subclass to instantiate
enum eActionName
{
	ACTION_NAME_NONE = -1,

	// Move Actions
	ACTION_NAME_MOVE,

	// Attack Actions
	ACTION_NAME_ATTACK,
	ACTION_NAME_ARROW,
	ACTION_NAME_METEOR,
	ACTION_NAME_GUARD,
	ACTION_NAME_HEAL,

	// Wait
	ACTION_NAME_WAIT,

	NUM_ACTION_NAMES
};


class PlayAction
{
public:
	//-----Public Methods-----

	PlayAction(eActionClass actionClass);

	// Virtual Methods
	virtual void Setup() = 0;
	virtual void Start() = 0;
	virtual void Update() = 0;
	virtual void RenderWorldSpace() const = 0;
	virtual void RenderScreenSpace() const = 0;
	virtual void Exit() = 0;

	// State Accessors
	bool IsReady() const;
	bool IsRunning() const;
	bool IsFinished() const;
	bool IsCancelled() const;

	eActionClass GetClass() const;

	// Factory functions for PlayActions
	static PlayAction* CreateAction(ActorController* controller, eActionName actionName);

	static eActionClass GetClassFromName(eActionName actionName);
	static eActionName GetActionNameFromText(const std::string& text);


protected:
	//-----Protected Data-----

	eActionState		m_actionState;
	eActionClass		m_actionClass;

};
