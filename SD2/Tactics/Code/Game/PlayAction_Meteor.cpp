/************************************************************************/
/* File: PlayAction_Meteor.cpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Implementation of the Meteor action class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Animator.hpp"
#include "Game/FlyoutText.hpp"
#include "Game/BoardState.hpp"
#include "Game/ActorController.hpp"
#include "Game/PlayAction_Meteor.hpp"
#include "Game/GameState_Playing.hpp"
#include "Game/PlayMode_SelectTile.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"

const int		PlayAction_Meteor::METEOR_AOE_RANGE = 1;
const Vector3	PlayAction_Meteor::METEOR_SPAWN_OFFSET = Vector3(0.f, 5.f, 0.f);
const float		PlayAction_Meteor::METEOR_DROP_SPEED = 2.5f;
const IntRange	PlayAction_Meteor::METEOR_CAST_RANGE = IntRange(0, 4);
const float		PlayAction_Meteor::METEOR_DROP_INTERVAL = 2.f;
const IntRange	PlayAction_Meteor::METEOR_DAMAGE_RANGE = IntRange(20, 30);
const float		PlayAction_Meteor::METEOR_WAIT_TIME = 50.f;


//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayAction_Meteor::PlayAction_Meteor(ActorController* actorController)
	: PlayAction(ACTION_CLASS_ATTACK)
	, m_controller(actorController)
{
}


//-----------------------------------------------------------------------------------------------
// Pushes a select tile mode for determining where to drop the meteor
//
void PlayAction_Meteor::Setup()
{
	// Find the attackable coords
	BoardState* boardState = Game::GetCurrentBoardState();
	Actor* actor = m_controller->GetActor();

	std::vector<IntVector3> attackableCoords = boardState->GetCoordsWithinManhattanDistance(actor->GetMapCoordinate(), METEOR_CAST_RANGE.min, METEOR_CAST_RANGE.max);

	// Push a new mode for target selection
	GameState_Playing* playState = Game::GetGameStatePlaying();

	TileSelectionCallback callback = std::bind(&PlayAction_Meteor::OnTileSelection, this, std::placeholders::_1, std::placeholders::_2);
	playState->PushMode(new PlayMode_SelectTile(m_controller, attackableCoords, callback));

	m_actionState = ACTION_STATE_READY;
}


//-----------------------------------------------------------------------------------------------
// Begins dropping the meteor from a fixed location above the target space
//
void PlayAction_Meteor::Start()
{
	m_stopwatch = new Stopwatch(Game::GetGameClock());
	m_stopwatch->SetTimer(METEOR_DROP_INTERVAL);

	Map* map = Game::GetCurrentBoardState()->GetMap();
	Actor* actor = m_controller->GetActor();

	Vector3 targetPosition = map->MapCoordsToWorldPosition(m_targetSpace);
	m_meteorPosition = targetPosition + METEOR_SPAWN_OFFSET;

	// Play the cast animation
	m_controller->GetActor()->GetAnimator()->Play("cast", PLAYMODE_CLAMP);

	// Turn the attacking actor to the target
	Vector3 attackDirection = (targetPosition - actor->GetWorldPosition()).GetNormalized();
	Vector2 attackDirection2D = attackDirection.xz();
	actor->SetOrientation(attackDirection2D.GetOrientationDegrees());

	m_actionState = ACTION_STATE_RUNNING;
}


//-----------------------------------------------------------------------------------------------
// Updates the meteor position, checking for when it lands to do the damage calculation
//
void PlayAction_Meteor::Update()
{
	m_meteorPosition.y -= Game::GetGameDeltaSeconds() * METEOR_DROP_SPEED;

	if (m_stopwatch->HasIntervalElapsed())
	{
		if (!m_damageStepStarted)
		{
			StartDamageStep();
		}
		else if (CleanUpFlyouts())
		{
			FinishDamageStep();
			m_actionState = ACTION_STATE_FINISHED;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the meteor at its current position
//
void PlayAction_Meteor::RenderWorldSpace() const
{
	if (!m_damageStepStarted)
	{
		Renderer* renderer = Renderer::GetInstance();
		renderer->BindTexture(0, "White");
		renderer->DrawCube(m_meteorPosition, Vector3::ONES, Rgba::ORANGE);
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
void PlayAction_Meteor::RenderScreenSpace() const
{
}


//-----------------------------------------------------------------------------------------------
// Cleans up before deletion
//
void PlayAction_Meteor::Exit()
{
	if (m_actionState == ACTION_STATE_FINISHED)
	{
		m_controller->GetActor()->AddToWaitTime(METEOR_WAIT_TIME);
		m_controller->SetControllerFlag(m_actionClass, true);
	}

	m_controller->GetActor()->GetAnimator()->Play("idle");

	for (int damageIndex = 0; damageIndex < (int) m_damageResults.size(); ++damageIndex)
	{
		Actor* actor = m_damageResults[damageIndex].m_actorHit;
		actor->TakeDamage(m_damageResults[damageIndex].m_amount);

		actor->GetAnimator()->Play("idle");
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the target location for the meteor
//
void PlayAction_Meteor::OnTileSelection(bool wasCancelled, const IntVector3& selectedLocation)
{
	if (wasCancelled)
	{
		m_actionState = ACTION_STATE_CANCELLED;
	}
	else
	{
		m_targetSpace = selectedLocation;
		m_actionState = ACTION_STATE_READY;
	}
}


//-----------------------------------------------------------------------------------------------
// Searches all tiles within a 3x3 square around the target tile for actors, and calculates the
// damage all the actors take - does not apply the damage until flyouts are done
//
void PlayAction_Meteor::StartDamageStep()
{
	BoardState* boardState = Game::GetCurrentBoardState();
	Map* map = boardState->GetMap();

	// Get all actors within the square
	for (int xCoord = m_targetSpace.x - METEOR_AOE_RANGE; xCoord <= m_targetSpace.x + METEOR_AOE_RANGE; ++xCoord)
	{
		for (int yCoord = m_targetSpace.y - METEOR_AOE_RANGE; yCoord <= m_targetSpace.y + METEOR_AOE_RANGE; ++yCoord)
		{
			IntVector3 currCoords = IntVector3(xCoord, yCoord, 0);
			if (map->Are2DCoordsValid(currCoords))
			{
				currCoords.z = map->GetHeightAtMapCoords(currCoords.xy());
				int heightDiff = AbsoluteValue(m_targetSpace.z - currCoords.z);

				if (heightDiff <= 2)
				{
					Actor* actor = boardState->GetActorAtCoords(currCoords.xy());
					if (actor != nullptr)
					{
						bool wasCrit = CheckRandomChance(m_controller->GetActor()->GetCritChance());
						int damageAmount = METEOR_DAMAGE_RANGE.GetRandomInRange();

						if (wasCrit)
						{
							damageAmount *= 2;
						}

						MeteorDamageInfo result = MeteorDamageInfo(damageAmount, wasCrit, actor);
						CreateFlyoutForResult(result);
						m_damageResults.push_back(result);

						actor->GetAnimator()->Play("hit", PLAYMODE_CLAMP);
					}
				}
			}
		}
	}

	m_damageStepStarted = true;
}


//-----------------------------------------------------------------------------------------------
// Deletes the finished flyouts, and returns true if there are no unfinished flyouts remaining
bool PlayAction_Meteor::CleanUpFlyouts()
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
// Applies the damage results to the actors
//
void PlayAction_Meteor::FinishDamageStep()
{
	for (int damageIndex = 0; damageIndex < (int) m_damageResults.size(); ++damageIndex)
	{
		Actor* actor = m_damageResults[damageIndex].m_actorHit;
		actor->TakeDamage(m_damageResults[damageIndex].m_amount);
	}
}


//-----------------------------------------------------------------------------------------------
// Creates floating texts for the resulting damage step
//
void PlayAction_Meteor::CreateFlyoutForResult(MeteorDamageInfo damageInfo)
{
	// Push a flyout text based on the result
	std::string descriptionText;
	if (damageInfo.m_wasCrit)		
	{ 
		descriptionText = "CRITICAL"; 
	}

	if (!IsStringNullOrEmpty(descriptionText))
	{
		FlyoutText* flyoutDescription = new FlyoutText(descriptionText, damageInfo.m_actorHit->GetWorldPosition() + Vector3(0.f, 5.f, 0.f), Rgba::WHITE);
		m_flyouts.push_back(flyoutDescription);
	}

	FlyoutText* flyoutNumber = new FlyoutText(std::to_string(damageInfo.m_amount), damageInfo.m_actorHit->GetWorldPosition() + Vector3(0.f, 3.0f, 0.f), Rgba::RED);
	m_flyouts.push_back(flyoutNumber);
}
