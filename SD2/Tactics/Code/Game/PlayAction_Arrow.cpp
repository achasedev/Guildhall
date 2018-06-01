/************************************************************************/
/* File: PlayAction_Arrow.cpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Implementation of the Arrow action
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/FlyoutText.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayAction_Arrow.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/PlayMode_SelectTile.hpp"

#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Trajectory.hpp"
#include "Engine/Renderer/Renderer.hpp"

// Constants
const float		PlayAction_Arrow::GRAVITY = 10.f;
const float		PlayAction_Arrow::LAUNCH_SPEED = 12.f;
const float		PlayAction_Arrow::ARROW_WAIT_TIME = 40.f;

const int		PlayAction_Arrow::NUM_STEP_SAMPLES = 100;
const IntRange	PlayAction_Arrow::ARROW_DAMAGE_RANGE = IntRange(15, 30);


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayAction_Arrow::PlayAction_Arrow(ActorController* actorController)
	: PlayAction(ACTION_CLASS_ATTACK)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Gets the list of selectable coordinates and pushes a select tile mode
//
void PlayAction_Arrow::Setup()
{
	// Find the attackable coords
	BoardState* boardState = Game::GetCurrentBoardState();
	Actor* actor = m_controller->GetActor();

	std::vector<IntVector3> selectableCoords = boardState->GetCoordsWithinManhattanDistance(actor->GetMapCoordinate(), 3, 6);

	// Push a new mode for target selection
	GameState_Playing* playState = Game::GetGameStatePlaying();
	TileSelectionCallback callback = std::bind(&PlayAction_Arrow::OnTileSelection, this, std::placeholders::_1, std::placeholders::_2);
	playState->PushMode(new PlayMode_SelectTile(m_controller, selectableCoords, callback));

	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Determines the trajectory of the arrow given the selected target and actor's current position
//
void PlayAction_Arrow::Start()
{
	CalculateTrajectory();

	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetTimer(m_launchDuration);

	// Play the animation
	m_controller->GetActor()->GetAnimator()->Play("attack", PLAYMODE_CLAMP);
	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Updates the arrow's position along the trajectory, terminating if the end is reach or the 
// arrow strikes an obstacle
//
void PlayAction_Arrow::Update()
{
	if (!m_stopwatch->HasIntervalElapsed())
	{
		UpdateTrajectory();
	}
	else
	{
		if (m_targetActor != nullptr && !m_damageStepStarted)
		{
			StartDamageStep();
		}

		bool flyoutsFinished = CleanUpFlyouts();
		if (flyoutsFinished)
		{
			FinishDamageStep();
			m_actionState = ACTION_STATE_FINISHED;
		}
	}

	if (m_controller->GetActor()->GetAnimator()->IsCurrentAnimationFinished())
	{
		m_controller->GetActor()->GetAnimator()->Play("idle");
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the projectile at its current position
//
void PlayAction_Arrow::RenderWorldSpace() const
{
	// For now, render a cube as the projectile
	if (!m_damageStepStarted)
	{
		Renderer* renderer = Renderer::GetInstance();
		renderer->BindTexture(0, "White");
		renderer->DrawCube(m_projectilePosition, 0.1f * Vector3::ONES, Rgba::YELLOW);
	}

	// Render the flyouts
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyouts.size(); ++flyoutIndex)
	{
		m_flyouts[flyoutIndex]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders screen space items
//
void PlayAction_Arrow::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Cleans up before deletion
//
void PlayAction_Arrow::Exit()
{
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->GetActor()->AddToWaitTime(40.f);
		m_controller->SetControllerFlag(m_actionClass, true);
	}

	m_controller->GetActor()->GetAnimator()->Play("idle");
	m_targetActor->GetAnimator()->Play("idle");

	delete m_stopwatch;
	m_stopwatch = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets the target location and if an actor is present, stores a reference to them
//
void PlayAction_Arrow::OnTileSelection(bool wasCancelled, const IntVector3& selectedLocation)
{
	if (wasCancelled)
	{
		m_actionState = ACTION_STATE_CANCELLED;
	}
	else
	{
		m_attackSpace = selectedLocation;
		BoardState* boardState = Game::GetCurrentBoardState();
		m_targetActor = boardState->GetActorAtCoords(m_attackSpace.xy());

		m_actionState = ACTION_STATE_READY;
	}
}


//-----------------------------------------------------------------------------------------------
// Calculates the trajectory of the arrow given the start space, target, and fixed launch speed
//
void PlayAction_Arrow::CalculateTrajectory()
{
	Map* map = Game::GetCurrentBoardState()->GetMap();
	Actor* actor = m_controller->GetActor();

	// Set up the end points
	m_startPosition = actor->GetWorldPosition() + Vector3(0.f, 0.75f, 0.f);
	m_endPosition = map->MapCoordsToWorldPosition(m_attackSpace) + Vector3(0.f, 0.5f, 0.f);

	// Get displacements
	m_launchDirectionXZ = (m_endPosition - m_startPosition);
	m_launchDirectionXZ.y = 0.f;
	float verticalDisplacement = m_endPosition.y - m_startPosition.y;

	// We vary launch angles - all bows fire the arrow at the same speed
	Vector2 launchAngles;
	bool solutionExists = Trajectory::CalculateLaunchAngles(launchAngles, GRAVITY, LAUNCH_SPEED, m_launchDirectionXZ.NormalizeAndGetLength(), verticalDisplacement);

	// Bow should be able to reach all selectable tile ranges
	GUARANTEE_OR_DIE(solutionExists, "Error: PlayAction_Arrow::SetupArrowTrajectory() couldn't find angles solution");
	
	m_launchVelocity = Vector2::MakeDirectionAtDegrees(launchAngles.y) * LAUNCH_SPEED;

	// Get flight time for termination check
	m_launchDuration = Trajectory::CalculateFlightTime(GRAVITY, m_launchVelocity.y, verticalDisplacement);

	// Turn the attacking actor to the target
	Vector3 attackDirection = (m_targetActor->GetWorldPosition() - actor->GetWorldPosition()).GetNormalized();
	Vector2 attackDirection2D = attackDirection.xz();
	actor->SetOrientation(attackDirection2D.GetOrientationDegrees());
}


//-----------------------------------------------------------------------------------------------
// Updates the arrow along the trajectory path
//
void PlayAction_Arrow::UpdateTrajectory()
{
	float timeElapsed = m_stopwatch->GetElapsedTime();
	Vector2 trajectoryOffset = Trajectory::EvaluateTrajectoryAtTime(GRAVITY, m_launchVelocity, timeElapsed);

	// Convert the offset from 2D trajectory space to world space
	Vector3 forwardOffset = trajectoryOffset.x * m_launchDirectionXZ;
	Vector3 upOffset = trajectoryOffset.y * Vector3::DIRECTION_UP;
	Vector3 worldOffset = forwardOffset + upOffset;

	// Update the projectile position
	m_projectilePosition = m_startPosition + worldOffset;

	// Check for termination
	Map* map = Game::GetCurrentBoardState()->GetMap();
	if (map->IsPositionInSolidBlock(m_projectilePosition))
	{
		m_actionState = ACTION_STATE_FINISHED;
	}
}


//-----------------------------------------------------------------------------------------------
// Calculates the damage taken and 
//
void PlayAction_Arrow::StartDamageStep()
{
	Actor* attackingActor = m_controller->GetActor();
	m_damageAmount = ARROW_DAMAGE_RANGE.GetRandomInRange();

	// Check for block, factoring in base block chance of defender, current block bonus (guard), and positional bonus
	float positionalBlockBonus = GetPositionalBlockChance();
	bool wasBlocked = CheckRandomChance(m_targetActor->GetBlockRate() + positionalBlockBonus);

	// Check for crit
	float positionalCritChance = GetPositionalCritChance();
	bool wasCrit = CheckRandomChance(attackingActor->GetCritChance() + positionalCritChance);

	// Push a flyout text based on the result
	std::string descriptionText;
	if (wasBlocked)
	{ 
		descriptionText = "BLOCKED";  
		m_damageAmount = 0;
	}
	else if (wasCrit)		
	{ 
		descriptionText = "CRITICAL"; 
		m_damageAmount *= 2;
	}

	if (!IsStringNullOrEmpty(descriptionText))
	{
		FlyoutText* flyoutDescription = new FlyoutText(descriptionText, m_targetActor->GetWorldPosition() + Vector3(0.f, 5.f, 0.f), Rgba::WHITE);
		m_flyouts.push_back(flyoutDescription);
	}

	FlyoutText* flyoutNumber = new FlyoutText(std::to_string(m_damageAmount), m_targetActor->GetWorldPosition() + Vector3(0.f, 3.0f, 0.f), Rgba::RED);
	m_flyouts.push_back(flyoutNumber);

	m_damageStepStarted = true;

	if (!wasBlocked && m_damageAmount > 0)
	{
		m_targetActor->GetAnimator()->Play("hit", PLAYMODE_CLAMP);
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all finished flyouts, and returns true if there are no unfinished flyouts left
//
bool PlayAction_Arrow::CleanUpFlyouts()
{
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyouts.size(); ++flyoutIndex)
	{
		FlyoutText* currFlyout = m_flyouts[flyoutIndex];
		if (currFlyout->IsFinished())
		{
			m_flyouts.erase(m_flyouts.begin() + flyoutIndex);
			delete currFlyout;
			flyoutIndex--;
		}
	}

	return (m_flyouts.size() == 0);
}


//-----------------------------------------------------------------------------------------------
// Deals damage to the target actor
//
void PlayAction_Arrow::FinishDamageStep()
{
	m_targetActor->TakeDamage(m_damageAmount);
}


//-----------------------------------------------------------------------------------------------
// Finds and returns the block chance gained from the attacker's position relative to the defender
//
float PlayAction_Arrow::GetPositionalBlockChance() const
{
	Actor* attackingActor = m_controller->GetActor();

	Vector3 directionToDefender = (m_targetActor->GetWorldPosition() - attackingActor->GetWorldPosition()).GetNormalized();

	Vector3 defenderForward = m_targetActor->GetWorldForward();

	float dotProduct = DotProduct(directionToDefender, defenderForward);

	float defendChance = 0.05f;	// Defend chance from the side
	float minDotToBeBehind = CosDegrees(45.f);

	if (dotProduct > minDotToBeBehind)
	{
		defendChance = 0.f;	// Mostly behind the defender
	}
	else if (dotProduct < -minDotToBeBehind)
	{
		defendChance = 0.20f; // Mostly in front of the defender
	}

	return defendChance;
}


//-----------------------------------------------------------------------------------------------
// Finds and returns the crit chance gained from the attacker's position relative to the defender
//
float PlayAction_Arrow::GetPositionalCritChance() const
{
	Actor* attackingActor = m_controller->GetActor();

	Vector3 directionToDefender = (m_targetActor->GetWorldPosition() - attackingActor->GetWorldPosition()).GetNormalized();

	Vector3 defenderForward = m_targetActor->GetWorldForward();

	float dotProduct = DotProduct(directionToDefender, defenderForward);

	float defendChance = 0.15f;	// Defend chance from the side
	float minDotToBeBehind = CosDegrees(45.f);

	if (dotProduct > minDotToBeBehind)
	{
		defendChance = 0.25f;	// Mostly behind the defender
	}
	else if (dotProduct < -minDotToBeBehind)
	{
		defendChance = 0.f; // Mostly in front of the defender
	}

	return defendChance;
}