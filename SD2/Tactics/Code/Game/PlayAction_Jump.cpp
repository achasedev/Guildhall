/************************************************************************/
/* File: PlayAction_Jump.cpp
/* Author: Andrew Chase
/* Date: March 16th, 2018
/* Description: Implementation of the jump action
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayAction_Jump.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Trajectory.hpp"

const float PlayAction_Jump::GRAVITY = 10.f;


//-----------------------------------------------------------------------------------------------
// Constructor - takes the actor (who is the start position) and the end position of the trajectory
//
PlayAction_Jump::PlayAction_Jump(Actor* actor, const Vector3& target)
	: PlayAction(ACTION_CLASS_NONE)
	, m_actor(actor)
	, m_endPosition(target)
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
}


//-----------------------------------------------------------------------------------------------
// Setup (unused)
//
void PlayAction_Jump::Setup()
{
	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Using the current actor's state, calculates the trajectory curve
//
void PlayAction_Jump::Start()
{
	// Get start position
	m_startPosition = m_actor->GetWorldPosition();

	// Find the launch velocity
	float verticalDisplacement = (m_endPosition.y - m_actor->GetWorldPosition().y);
	float apexHeight = ClampFloat(verticalDisplacement, 0.f, (float) m_actor->GetJumpHeight()) + 0.5f;
	m_launchVelocity = Trajectory::CalculateLaunchVelocity(GRAVITY, apexHeight, 1.0f, verticalDisplacement);

	// Launch duration, used to determine when the trajectory is finished
	m_launchDuration = Trajectory::CalculateFlightTime(GRAVITY, m_launchVelocity.y, verticalDisplacement);

	// Make sure the actor faces the right direction
	Vector3 direction = (m_endPosition - m_startPosition);
	Vector2 direction2D = Vector2(direction.x, direction.z);
	m_actor->SetOrientation(direction2D.GetOrientationDegrees());

	// Setup time-based things
	m_actor->GetAnimator()->Play("jump", PLAYMODE_CLAMP);
	m_stopwatch->SetTimer(m_launchDuration);

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Updates the actor's position along the trajectory
//
void PlayAction_Jump::Update()
{
	float elapsedTime = m_stopwatch->GetElapsedTime();

	// Set the actor position, flagging if we're done
	if (m_stopwatch->HasIntervalElapsed())
	{
		m_actor->SetWorldPosition(m_endPosition);
		m_actionState = ACTION_STATE_FINISHED;
	}
	else 
	{
		// Get the delta position of the actor relative to the start of the trajectory
		Vector2 trajectoryOffset = Trajectory::EvaluateTrajectoryAtTime(GRAVITY, m_launchVelocity, elapsedTime);

		// Convert the offset from 2D trajectory space to world space
		Vector3 forwardOffset = trajectoryOffset.x * m_actor->GetWorldForward();
		Vector3 upOffset = trajectoryOffset.y * Vector3::DIRECTION_UP;
		Vector3 worldOffset = forwardOffset + upOffset;

		m_actor->SetWorldPosition(m_startPosition + worldOffset);
	}

	// Update the camera
	GameState_Playing* playState = Game::GetGameStatePlaying();
	playState->TranslateCamera(m_actor->GetWorldPosition());
}


//-----------------------------------------------------------------------------------------------
// Render in world space (for jump effects)
//
void PlayAction_Jump::RenderWorldSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Render in screen space (for UI)
//
void PlayAction_Jump::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Resets the actor's animation back to idle and cleans up
//
void PlayAction_Jump::Exit()
{
	m_actor->GetAnimator()->Play("idle");

	delete m_stopwatch;
	m_stopwatch = nullptr;
}
