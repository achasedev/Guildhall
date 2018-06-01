/************************************************************************/
/* File: PlayAction_Attack.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the attack PlayAction
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Animator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BoardState.hpp"
#include "Game/FlyoutText.hpp"
#include "Game/ActorController.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/PlayAction_Attack.hpp"
#include "Game/PlayMode_SelectTile.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - Takes the controller for the acting actor
//
PlayAction_Attack::PlayAction_Attack(ActorController* actorController)
	: PlayAction(ACTION_CLASS_ATTACK)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Initializes the action by pushing a select tile PlayMode to determine where to attack
//
void PlayAction_Attack::Setup()
{
	// Find the attackable coords
	BoardState* boardState = Game::GetCurrentBoardState();
	Actor* actor = m_controller->GetActor();

	std::vector<IntVector3> attackableCoords = boardState->GetAttackableCoords(actor->GetMapCoordinate());

	// Push a new mode for target selection
	GameState_Playing* playState = Game::GetGameStatePlaying();

	TileSelectionCallback callback = std::bind(&PlayAction_Attack::OnTileSelection, this, std::placeholders::_1, std::placeholders::_2);
	playState->PushMode(new PlayMode_SelectTile(m_controller, attackableCoords, callback));

	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Called when an attack begins updating
// 
void PlayAction_Attack::Start()
{
	BoardState* boardState = Game::GetCurrentBoardState();
	m_defendingActor = boardState->GetActorAtCoords(m_attackSpace.xy());

	if (m_defendingActor != nullptr)
	{
		StartDamageStep();
	}

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Performs the damage step
//
void PlayAction_Attack::Update()
{
	if (CleanUpFlyouts())
	{
		if (m_defendingActor != nullptr)
		{
			FinishDamageStep();
		}

		m_controller->GetActor()->AddToWaitTime(40.f);
		m_actionState = ACTION_STATE_FINISHED;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the attack effects in the world
//
void PlayAction_Attack::RenderWorldSpace() const
{
	// Render flyouts
	for (int flyoutIndex = 0; flyoutIndex < (int) m_flyoutTexts.size(); ++flyoutIndex)
	{
		m_flyoutTexts[flyoutIndex]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
// Renders any screen space attack elements
//
void PlayAction_Attack::RenderScreenSpace() const
{

}


//-----------------------------------------------------------------------------------------------
// Called before deletion of the action
//
void PlayAction_Attack::Exit()
{
	// Tell the controller we attacked
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->SetControllerFlag(m_actionClass, true);
	}
}


//-----------------------------------------------------------------------------------------------
// Called after an attack tile was selected (or if selection was cancelled)
// 
void PlayAction_Attack::OnTileSelection(bool wasCancelled, const IntVector3& selectedLocation)
{
	if (wasCancelled)
	{
		m_actionState = ACTION_STATE_CANCELLED;
	}
	else
	{
		m_attackSpace = selectedLocation;
		m_actionState = ACTION_STATE_READY;
	}
}


//-----------------------------------------------------------------------------------------------
// Does the damage calculation and pushes the flyout text, does not apply the damage yet
//
void PlayAction_Attack::StartDamageStep()
{
	Actor* attackingActor = m_controller->GetActor();
	m_damageAmount = attackingActor->GetStrength();

	// Check for block - factor in base defend chance, defend bonuses of this actor, and the positional bonus
	float positionalDefendChance = GetPositionalBlockChance();
	bool wasBlocked = CheckRandomChance(m_defendingActor->GetBlockRate() + positionalDefendChance);

	// Check for crit
	float positionalCritChance = GetPositionalCritChance();
	bool wasCrit = CheckRandomChance(attackingActor->GetCritChance() + positionalCritChance);

	// Turn the attacking actor to the target
	Vector3 attackDirection = (m_defendingActor->GetWorldPosition() - attackingActor->GetWorldPosition()).GetNormalized();
	Vector2 attackDirection2D = attackDirection.xz();
	attackingActor->SetOrientation(attackDirection2D.GetOrientationDegrees());

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
		FlyoutText* flyoutDescription = new FlyoutText(descriptionText, m_defendingActor->GetWorldPosition() + Vector3(0.f, 5.f, 0.f), Rgba::WHITE);
		m_flyoutTexts.push_back(flyoutDescription);
	}

	FlyoutText* flyoutNumber = new FlyoutText(std::to_string(m_damageAmount), m_defendingActor->GetWorldPosition() + Vector3(0.f, 3.0f, 0.f), Rgba::RED);
	m_flyoutTexts.push_back(flyoutNumber);

	attackingActor->GetAnimator()->Play("attack", PLAYMODE_CLAMP);

	if (!wasBlocked && m_damageAmount > 0)
	{
		m_defendingActor->GetAnimator()->Play("hit", PLAYMODE_CLAMP);
	}
}


//-----------------------------------------------------------------------------------------------
// Deletes all finished flyouts, and returns true if no unfinished flyouts remain
//
bool PlayAction_Attack::CleanUpFlyouts()
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


//-----------------------------------------------------------------------------------------------
// Applies the calculated damage to the defending actor
//
void PlayAction_Attack::FinishDamageStep()
{
	m_defendingActor->TakeDamage(m_damageAmount);
	m_controller->GetActor()->GetAnimator()->Play("idle");
	m_defendingActor->GetAnimator()->Play("idle");
}


//-----------------------------------------------------------------------------------------------
// Finds and returns the block chance gained from the attacker's position relative to the defender
//
float PlayAction_Attack::GetPositionalBlockChance() const
{
	Actor* attackingActor = m_controller->GetActor();

	Vector3 directionToDefender = (m_defendingActor->GetWorldPosition() - attackingActor->GetWorldPosition()).GetNormalized();

	Vector3 defenderForward = m_defendingActor->GetWorldForward();

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
float PlayAction_Attack::GetPositionalCritChance() const
{
	Actor* attackingActor = m_controller->GetActor();

	Vector3 directionToDefender = (m_defendingActor->GetWorldPosition() - attackingActor->GetWorldPosition()).GetNormalized();

	Vector3 defenderForward = m_defendingActor->GetWorldForward();

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
