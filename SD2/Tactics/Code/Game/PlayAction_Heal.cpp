/************************************************************************/
/* File: PlayAction_Heal.cpp
/* Author: Andrew Chase
/* Date: March 23rd, 2018
/* Description: Implementation of the Heal action class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/FlyoutText.hpp"
#include "Game/BoardState.hpp"
#include "PlayMode_SelectTile.hpp"
#include "Game/PlayAction_Heal.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameState_Playing.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

// Constants
const float		PlayAction_Heal::HEAL_WAIT_TIME = 40.f;
const IntRange	PlayAction_Heal::HEAL_RANGE = IntRange(0, 3);
const IntRange  PlayAction_Heal::HEAL_AMOUNT = IntRange(20, 30);


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayAction_Heal::PlayAction_Heal(ActorController* actorController)
	: PlayAction(ACTION_CLASS_ATTACK)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Pushes a select tile mode to determine who to heal
//
void PlayAction_Heal::Setup()
{
	// Find the healable coords
	BoardState* boardState = Game::GetCurrentBoardState();
	Actor* actor = m_controller->GetActor();

	std::vector<IntVector3> selectableCoords = boardState->GetCoordsWithinManhattanDistance(actor->GetMapCoordinate(), 3, 6);

	// Push a new mode for target selection
	GameState_Playing* playState = Game::GetGameStatePlaying();
	TileSelectionCallback callback = std::bind(&PlayAction_Heal::OnTileSelection, this, std::placeholders::_1, std::placeholders::_2);
	playState->PushMode(new PlayMode_SelectTile(m_controller, selectableCoords, callback));

	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Sets up the actor's orientation and starts the animation
//
void PlayAction_Heal::Start()
{
	Actor* actor = m_controller->GetActor();
	// Play the animation
	actor->GetAnimator()->Play("cast", PLAYMODE_CLAMP);

	// Turn the attacking actor to the target
	Vector3 attackDirection = (m_targetActor->GetWorldPosition() - actor->GetWorldPosition()).GetNormalized();
	Vector2 attackDirection2D = attackDirection.xz();
	actor->SetOrientation(attackDirection2D.GetOrientationDegrees());

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Performs the heal step and creates the flyout texts
//
void PlayAction_Heal::Update()
{
	if (m_targetActor != nullptr && !m_healStepStarted)
	{
		StartHealStep();
	}
	else
	{
		bool flyoutsFinished = CleanUpFlyouts();
		if (flyoutsFinished)
		{
			FinishHealStep();
			m_actionState = ACTION_STATE_FINISHED;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the flyouts
//
void PlayAction_Heal::RenderWorldSpace() const
{
	// Render the flyouts
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyouts.size(); ++flyoutIndex)
	{
		m_flyouts[flyoutIndex]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders screen space elements (unused)
//
void PlayAction_Heal::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Resets the actor's animations and signals the controller an "attack" class action was used
//
void PlayAction_Heal::Exit()
{
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->SetControllerFlag(m_actionClass, true);
	}

	m_controller->GetActor()->GetAnimator()->Play("idle");
	m_targetActor->GetAnimator()->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Callback called when a tile is selected
//
void PlayAction_Heal::OnTileSelection(bool wasCancelled, const IntVector3& selectedLocation)
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
// Calculates the heal amount and pushes a flyout text
//
void PlayAction_Heal::StartHealStep()
{
	m_healAmount = HEAL_AMOUNT.GetRandomInRange();

	// Check for crit
	bool wasCrit = CheckRandomChance(m_controller->GetActor()->GetCritChance());

	// Push a flyout text based on the result
	std::string descriptionText;
	if (wasCrit)		
	{ 
		descriptionText = "CRITICAL"; 
		m_healAmount *= 2;
	}

	if (!IsStringNullOrEmpty(descriptionText))
	{
		FlyoutText* flyoutDescription = new FlyoutText(descriptionText, m_targetActor->GetWorldPosition() + Vector3(0.f, 5.f, 0.f), Rgba::WHITE);
		m_flyouts.push_back(flyoutDescription);
	}

	FlyoutText* flyoutNumber = new FlyoutText(std::to_string(m_healAmount), m_targetActor->GetWorldPosition() + Vector3(0.f, 3.0f, 0.f), Rgba::LIGHT_GREEN);
	m_flyouts.push_back(flyoutNumber);

	m_targetActor->GetAnimator()->Play("jump", PLAYMODE_CLAMP);

	m_healStepStarted = true;
}


//-----------------------------------------------------------------------------------------------
// Deletes any finished flyout texts, returning true if there are no unfinishes flyouts left
//
bool PlayAction_Heal::CleanUpFlyouts()
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
// Applies the heal amount to the target actor
//
void PlayAction_Heal::FinishHealStep()
{
	m_targetActor->TakeDamage(-m_healAmount);
}
