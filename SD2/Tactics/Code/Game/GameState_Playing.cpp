/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Menu.hpp"
#include "Game/Actor.hpp"
#include "Game/Animator.hpp"
#include "Game/PlayMode.hpp"
#include "Game/IsoSprite.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayAction.hpp"
#include "Game/AnimationSet.hpp"
#include "Game/PlayMode_Pause.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/PlayMode_Default.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor - main function is to set up the orbit camera
//
GameState_Playing::GameState_Playing(BoardStateDefinition* boardStateDefinition)
{
	m_gameCamera = new OrbitCamera();
	m_gameCamera->SetProjectionOrtho(10.f, 0.f, 200.f);
	
	// Set the orbit camera's render targets to be the default camera targets
	Renderer* renderer = Renderer::GetInstance();

	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetRadiusLimits(1.f, 100.f);
	m_gameCamera->SetAzimuthLimits(20.f, 80.f);
	m_gameCamera->SetSphericalCoordinates(30.f, 45.f, 45.f);
	m_gameCamera->SetOrthoSizeLimits(1.f, 20.f);

	// Initialize the map and the actors
	SetBoardStateDefinition(boardStateDefinition);

	// Initialize the default mode
	m_modeStack.push(new PlayMode_Default());
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	DeleteActionsAndModes();

	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_boardState;
	m_boardState = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets up the boardstate from the given boardstate definition
//
void GameState_Playing::SetBoardStateDefinition(BoardStateDefinition* definition)
{
	m_boardStateDefinition = definition;
}


//-----------------------------------------------------------------------------------------------
// Sets up the Encounter when this state is entered
//
void GameState_Playing::Enter()
{
	m_boardState = new BoardState(m_boardStateDefinition);
	m_boardState->InitializeMapFromDefinition();
	m_boardState->SpawnActorsFromDefinition();
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input relevant to the mode, only if no actions are running
//
void GameState_Playing::ProcessInput()
{
	bool isPaused = IsTopModePause();

	if (isPaused || !HasRunningActions())
	{
		PlayMode* currMode = m_modeStack.top();
		currMode->ProcessInput();
	}

	// Check to pause the game, ensuring we don't pause when we are already paused
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = InputSystem::GetPlayerOneController();

	if (!isPaused)
	{
		if (controller.WasButtonJustPressed(XBOX_BUTTON_START) || input->WasKeyJustPressed(InputSystem::KEYBOARD_F7) || input->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
		{
			PushMode(new PlayMode_Pause());
		}
		else 
		{
			UpdateCameraOnInput();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Updates any actions if present, otherwise updates the mode on the top of the stack
//
void GameState_Playing::Update()
{
	m_boardState->Update();

	if (HasRunningActions())
	{
		UpdateActions();
	}
	else
	{
		UpdateModes();
	}

	// Clean up and exit actions and modes
	FinishActionsAndModes();

	// Rotate the camera if necessary
	if (m_isCameraRotating)
	{
		LerpCameraToRotation(m_rotationTarget);
	}

	m_boardState->DeleteDeadActors();
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	RenderWorldSpace();
	RenderScreenSpace();
}


//-----------------------------------------------------------------------------------------------
// Returns the BoardState of the game
//
BoardState* GameState_Playing::GetBoardState() const
{
	return m_boardState;
}


//-----------------------------------------------------------------------------------------------
// Returns true if this state has actions in the action queue
//
bool GameState_Playing::HasRunningActions() const
{
	return m_actionQueue.size() > 0;
}


//-----------------------------------------------------------------------------------------------
// Calls Update() on the action at the front of the action queue
//
void GameState_Playing::UpdateActions()
{
	// Get the action at the front of the queue and update
	PlayAction* currAction = m_actionQueue.front();
	if (!currAction->IsRunning())
	{
		currAction->Start();
	}

	currAction->Update();
}


//-----------------------------------------------------------------------------------------------
// Calls Update() on the mode at the top of the mode stack
//
void GameState_Playing::UpdateModes()
{
	PlayMode* currMode = m_modeStack.top();

	if (!currMode->IsStarted())
	{
		currMode->Start();
	}

	currMode->Update();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the top mode is a pause mode - used to allow bypassing of the action queue
//
bool GameState_Playing::IsTopModePause() const
{
	PlayMode* currMode = m_modeStack.top();

	PlayMode_Pause* pauseMode = dynamic_cast<PlayMode_Pause*>(currMode);
	bool isPaused = (pauseMode != nullptr);	// Successful cast == mode is a pause mode

	return isPaused;
}


//-----------------------------------------------------------------------------------------------
// Checks the front and top of the action queue/mode stack for finished actions and modes, and 
// removes them
//
void GameState_Playing::FinishActionsAndModes()
{
	// Actions
	if (HasRunningActions())
	{
		PlayAction* frontAction = m_actionQueue.front();
		if (frontAction->IsFinished())
		{
			frontAction->Exit();	

			m_actionQueue.pop();
			delete frontAction;
		}
		else if (frontAction->IsCancelled())
		{
			m_actionQueue.pop();
			delete frontAction;
		}
	}

	// Modes
	PlayMode* topMode = m_modeStack.top();
	if (topMode->IsFinished())
	{
		topMode->Exit();
		m_modeStack.pop();

		delete topMode;
	}
}


//-----------------------------------------------------------------------------------------------
// Removes and deletes all actions and modes in both the action queue and the mode stack
//
void GameState_Playing::DeleteActionsAndModes()
{
	// Delete all actions
	while ((int) m_actionQueue.size() > 0)
	{
		PlayAction* frontAction = m_actionQueue.front();
		
		m_actionQueue.pop();
		delete frontAction;
	}

	// Delete all modes
	while ((int) m_modeStack.size() > 0)
	{
		PlayMode* topMode = m_modeStack.top();

		m_modeStack.pop();
		delete topMode;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders all world space elements for the playing state
//
void GameState_Playing::RenderWorldSpace() const
{
	// Ensure we're using the OrbitCamera for the following renders
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(m_gameCamera);

	// Render the map
	m_boardState->RenderMap();

	if (HasRunningActions())
	{
		PlayAction* action = m_actionQueue.front();
		action->RenderWorldSpace();
	} 

	PlayMode* mode = m_modeStack.top();
	mode->RenderWorldSpace();

	// Ensure we render actors after the tiles
	m_boardState->RenderActors();
}


//-----------------------------------------------------------------------------------------------
// Renders all screen space elements for the playing state
//
void GameState_Playing::RenderScreenSpace() const
{
	// Ensure we're using the UI for the following renders
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	renderer->DisableDepth();

	if (HasRunningActions())
	{
		PlayAction* action = m_actionQueue.front();
		action->RenderScreenSpace();
	} 

	PlayMode* mode = m_modeStack.top();
	mode->RenderScreenSpace();

	// Render other screen stuff here
}


//-----------------------------------------------------------------------------------------------
// Adds the given action to the end of the action queue
//
void GameState_Playing::EnqueueAction(PlayAction* action)
{
	m_actionQueue.emplace(action);
}


//-----------------------------------------------------------------------------------------------
// Pushes the given mode to the top of the mode stack
//
void GameState_Playing::PushMode(PlayMode* mode)
{
	m_modeStack.emplace(mode);
}


//-----------------------------------------------------------------------------------------------
// Returns the game camera
//
OrbitCamera* GameState_Playing::GetGameCamera()
{
	return m_gameCamera;
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's target to move towards target
//
void GameState_Playing::TranslateCamera(const Vector3& target)
{
	Vector3 currTarget = m_gameCamera->GetTarget();
	if (currTarget == target)
	{
		return;
	}

	Vector3 direction = (target - currTarget).GetNormalized();
	Vector3 nextTarget = currTarget + direction * 5.0f * Game::GetGameDeltaSeconds();

	float distance = (nextTarget - target).GetLength();
	if (distance < 0.08f)
	{
		nextTarget = target;
	}

	m_gameCamera->SetTarget(nextTarget);
}


//-----------------------------------------------------------------------------------------------
// Updates the camera's rotation to rotate towards the rotation target
//
void GameState_Playing::LerpCameraToRotation(float rotationTarget)
{
	float currRotation = m_gameCamera->GetRotation();
	float nextRotation = TurnToward(currRotation, rotationTarget, 180.f * Game::GetGameDeltaSeconds());

	m_gameCamera->SetRotation(nextRotation);
}


//-----------------------------------------------------------------------------------------------
// Updates the camera position based on input
//
void GameState_Playing::UpdateCameraOnInput()
{
	XboxController& controller = InputSystem::GetPlayerOneController();
	float deltaTime = Game::GetGameDeltaSeconds();

	// Update rotation angles using right stick
	Vector2 stickInput = controller.GetCorrectedStickPosition(XBOX_STICK_RIGHT);
	if (stickInput != Vector2::ZERO)
	{
		m_isCameraRotating = false;
	}

	m_gameCamera->RotateHorizontally(stickInput.x * deltaTime * 120.f);
	m_gameCamera->RotateVertically(-stickInput.y * deltaTime * 80.f);


	//--------------------BONUS - Snapping angle based on bumper clicks--------------------
	float currRotation = m_gameCamera->GetRotation();
	float nearestInterCardinalAngle = GetNearestInterCardinalAngle(currRotation);

	if (controller.WasButtonJustPressed(XBOX_BUTTON_LB))
	{
		if (nearestInterCardinalAngle >= currRotation)
		{
			m_rotationTarget = nearestInterCardinalAngle - 90.f;
		}
		else
		{
			m_rotationTarget = nearestInterCardinalAngle;
		}

		m_isCameraRotating = true;
	}
	else if (controller.WasButtonJustPressed(XBOX_BUTTON_RB))
	{
		if (nearestInterCardinalAngle > currRotation)
		{
			m_rotationTarget = nearestInterCardinalAngle;
		}
		else
		{
			m_rotationTarget = nearestInterCardinalAngle + 90.f;
		}

		m_isCameraRotating = true;
	}
	//---------------------------------------------------------------------------------------

	// Update ortho size
	float leftTrigger = controller.GetTriggerValue(XBOX_TRIGGER_LEFT);
	float rightTrigger = controller.GetTriggerValue(XBOX_TRIGGER_RIGHT);
	float sizeModifier = (leftTrigger - rightTrigger) * deltaTime * 30.f;

	m_gameCamera->AdjustOrthoSize(sizeModifier);
}
