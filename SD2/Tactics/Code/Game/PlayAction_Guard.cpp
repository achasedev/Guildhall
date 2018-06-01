/************************************************************************/
/* File: PlayAction_Guard.cpp
/* Author: Andrew Chase
/* Date: March 21st, 2018
/* Description: Implementation of the Guard action class
/************************************************************************/
#include "Game/Animator.hpp"
#include "Game/FlyoutText.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayAction_Guard.hpp"

// Amount added to block rate
const float PlayAction_Guard::GUARD_BLOCK_BONUS = 0.5f;
const float PlayAction_Guard::GUARD_WAIT_TIME = 40.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayAction_Guard::PlayAction_Guard(ActorController* actorController)
	: PlayAction(ACTION_CLASS_ATTACK)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Setup - Unused
//
void PlayAction_Guard::Setup()
{
	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Pushes the guard effect, and plays the animation
//
void PlayAction_Guard::Start()
{
	// Push a flyout text
	FlyoutText* guardFlyout = new FlyoutText("GUARD", m_controller->GetActor()->GetWorldPosition() + Vector3(0.f, 3.0f, 0.f), Rgba::LIGHT_BLUE);
	m_flyoutTexts.push_back(guardFlyout);

	m_controller->GetActor()->GetAnimator()->Play("jump", PLAYMODE_CLAMP);

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Checks if the flyout text is done, and if so marks as finished
//
void PlayAction_Guard::Update()
{
	if (CleanUpFlyouts())
	{
		Actor* actor = m_controller->GetActor();
		actor->AddToBlockRateBonus(GUARD_BLOCK_BONUS);
		actor->AddToWaitTime(GUARD_WAIT_TIME);
		m_actionState = ACTION_STATE_FINISHED;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the flyouts
//
void PlayAction_Guard::RenderWorldSpace() const
{
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyoutTexts.size(); ++flyoutIndex)
	{
		m_flyoutTexts[flyoutIndex]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders screenspace elements
//
void PlayAction_Guard::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Called before deletion
//
void PlayAction_Guard::Exit()
{
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->SetControllerFlag(m_actionClass, true);
	}

	m_controller->GetActor()->GetAnimator()->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Deletes all finished flyouts, and returns true if no unfinished flyouts remain
//
bool PlayAction_Guard::CleanUpFlyouts()
{
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyoutTexts.size(); ++flyoutIndex)
	{
		FlyoutText* currFlyout = m_flyoutTexts[flyoutIndex];
		if (currFlyout->IsFinished())
		{
			m_flyoutTexts.erase(m_flyoutTexts.begin() + flyoutIndex);
			delete currFlyout;
		}
	}

	return (m_flyoutTexts.size() == 0);
}
