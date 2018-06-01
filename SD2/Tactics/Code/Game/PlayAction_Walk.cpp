/************************************************************************/
/* File: PlayAction_Walk.cpp
/* Author: Andrew Chase
/* Date: March 16th, 2018
/* Description: Implementation of the walk action class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/PlayAction_Walk.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Renderer/OrbitCamera.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - takes the actor and a position to walk to
//
PlayAction_Walk::PlayAction_Walk(Actor* actor, const Vector3& targetPosition)
	: PlayAction(ACTION_CLASS_NONE)
	, m_actor(actor)
	, m_targetPosition(targetPosition)
{
}


//-----------------------------------------------------------------------------------------------
// Setup (unused)
//
void PlayAction_Walk::Setup()
{
	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Sets the actor's orientation and animation for the walk
//
void PlayAction_Walk::Start()
{
	// Set the actor to face the direction of translation
	Vector3 actorPosition = m_actor->GetWorldPosition();
	Vector3 moveDirection = (m_targetPosition - actorPosition).GetNormalized();

	m_actor->SetOrientation(Vector2(moveDirection.x, moveDirection.z).GetOrientationDegrees());
	m_actor->GetAnimator()->Play("walk", PLAYMODE_LOOP);

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Translates the actor towards the target, stopping when within a small distance
//
void PlayAction_Walk::Update()
{
	Vector3 actorPosition = m_actor->GetWorldPosition();
	Vector3 moveDirection = (m_targetPosition - actorPosition).GetNormalized();

	float deltaTime = Game::GetGameDeltaSeconds();
	Vector3 nextPosition = actorPosition + moveDirection * 2.5f * deltaTime;

	// Check if we're close to the position, if so snap to it
	float distance = (m_targetPosition - nextPosition).GetLength();

	if (distance < 0.1f)
	{
		nextPosition = m_targetPosition;
		m_actionState = ACTION_STATE_FINISHED;
	}

	// Update the position
	m_actor->SetWorldPosition(nextPosition);

	// Update the camera
	GameState_Playing* playState = Game::GetGameStatePlaying();
	playState->TranslateCamera(m_actor->GetWorldPosition());
}


//-----------------------------------------------------------------------------------------------
// Renders in world space
//
void PlayAction_Walk::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Renders in screen space
//
void PlayAction_Walk::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Sets the actor back to an idle animation, maintaining their last orientation
//
void PlayAction_Walk::Exit()
{
	m_actor->GetAnimator()->Play("idle");
}
