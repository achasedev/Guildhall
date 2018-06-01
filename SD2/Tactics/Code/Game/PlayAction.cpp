/************************************************************************/
/* File: PlayMode.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the PlayAction base class
/************************************************************************/
#include "Game/PlayAction.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayAction_Move.hpp"
#include "Game/PlayAction_Wait.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayAction_Heal.hpp"
#include "Game/PlayAction_Arrow.hpp"
#include "Game/PlayAction_Guard.hpp"
#include "Game/PlayAction_Attack.hpp"
#include "Game/PlayAction_Meteor.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor - Takes the controller for the acting actor and the enum for the subclass
//
PlayAction::PlayAction(eActionClass actionClass)
	: m_actionClass(actionClass)
	, m_actionState(ACTION_STATE_NOT_READY)
{
}


//-----------------------------------------------------------------------------------------------
// Returns true if this Action has been setup
//
bool PlayAction::IsReady() const
{
	return (m_actionState != ACTION_STATE_NOT_READY);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the action has run Start() and is currently in the process of updating
//
bool PlayAction::IsRunning() const
{
	return (m_actionState == ACTION_STATE_RUNNING);
}


//-----------------------------------------------------------------------------------------------
// Returns true if the action has completed its task and is flagged as done
//
bool PlayAction::IsFinished() const
{
	return (m_actionState == ACTION_STATE_FINISHED);

}


//-----------------------------------------------------------------------------------------------
// Returns true if the action has been cancelled by the player (didn't complete the task)
//
bool PlayAction::IsCancelled() const
{
	return (m_actionState == ACTION_STATE_CANCELLED);
}


//-----------------------------------------------------------------------------------------------
// Returns the class of this action
//
eActionClass PlayAction::GetClass() const
{
	return m_actionClass;
}


//-----------------------------------------------------------------------------------------------
// Constructs and returns a PlayAction subclass of the given name
//
PlayAction* PlayAction::CreateAction(ActorController* controller, eActionName actionName)
{
	switch (actionName)
	{
	case ACTION_NAME_ATTACK:
		return new PlayAction_Attack(controller);
		break;
	case ACTION_NAME_MOVE:
		return new PlayAction_Move(controller);
		break;
	case ACTION_NAME_WAIT:
		return new PlayAction_Wait(controller);
		break;
	case ACTION_NAME_ARROW:
		return new PlayAction_Arrow(controller);
		break;
	case ACTION_NAME_METEOR:
		return new PlayAction_Meteor(controller);
		break;
	case ACTION_NAME_HEAL:
		return new PlayAction_Heal(controller);
		break;
	case ACTION_NAME_GUARD:
		return new PlayAction_Guard(controller);
		break;
	default:
		ERROR_AND_DIE("Error: PlayAction::CreateAction() received an unknown action name.");
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the class of the action given by the name
//
eActionClass PlayAction::GetClassFromName(eActionName actionName)
{
	switch (actionName)
	{
	case ACTION_NAME_MOVE:
		return ACTION_CLASS_MOVE;
		break;
	case ACTION_NAME_ATTACK:
	case ACTION_NAME_ARROW:
	case ACTION_NAME_METEOR:
	case ACTION_NAME_GUARD:
	case ACTION_NAME_HEAL:
		return ACTION_CLASS_ATTACK;
		break;
	case ACTION_NAME_WAIT:
		return ACTION_CLASS_WAIT;
		break;
	default:
		return ACTION_CLASS_NONE;
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Converts the string representation of an action name into the appropriate enumeration
//
eActionName PlayAction::GetActionNameFromText(const std::string& text)
{
	if		(text == "attack")		{ return ACTION_NAME_ATTACK; }
	else if (text == "move")		{ return ACTION_NAME_MOVE;   }
	else if (text == "wait" )		{ return ACTION_NAME_WAIT;   }
	else if (text == "arrow")		{ return ACTION_NAME_ARROW;  }
	else if (text == "meteor")		{ return ACTION_NAME_METEOR; }
	else if (text == "heal")		{ return ACTION_NAME_HEAL;	 } 
	else if (text == "guard")		{ return ACTION_NAME_GUARD;	 } 
	else
	{
		ERROR_AND_DIE(Stringf("Error: PlayAction::GetActionNameFromText() received bad name %s", text.c_str()));
	}
}
