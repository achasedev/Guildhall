/************************************************************************/
/* File: PlayMode.cpp
/* Author: Andrew Chase
/* Date: March 12th, 2018
/* Description: Implementation of the PlayMode base class
/************************************************************************/
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/PlayMode.hpp"
#include "Game/BoardState.hpp"
#include "Game/GameCommon.hpp"
#include "Game/ActorController.hpp"

#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/OrbitCamera.hpp"

// AABB2 in screenspace for drawing hover data
AABB2 PlayMode::s_hoverBounds;
AABB2 PlayMode::s_turnOrderBounds;

//-----------------------------------------------------------------------------------------------
// Only constructor, takes the controller of the currently acting actor
//
PlayMode::PlayMode(ActorController* controller)
	: m_controller(controller)
{
	// Set the cursor on the current actor
	if (controller != nullptr)
	{
		Actor* actor = m_controller->GetActor();
		m_cursorCoords = actor->GetMapCoordinate();
	}

	Window* window = Window::GetInstance();
	s_hoverBounds = AABB2(Vector2(0.f, Renderer::UI_ORTHO_HEIGHT - 150.f), Vector2(200.f * window->GetWindowAspect(), Renderer::UI_ORTHO_HEIGHT));

	float orthoWidth = Renderer::UI_ORTHO_HEIGHT * window->GetWindowAspect();
	s_turnOrderBounds = AABB2(Vector2(orthoWidth - 150.f * window->GetWindowAspect(), Renderer::UI_ORTHO_HEIGHT - 500.f), Vector2(orthoWidth, Renderer::UI_ORTHO_HEIGHT));
}


//-----------------------------------------------------------------------------------------------
// Called when the mode first begins updating
//
void PlayMode::Start()
{
	// Update the actor we're currently hovering over
	m_hoverActor = Game::GetCurrentBoardState()->GetActorAtCoords(m_cursorCoords.xy());
	m_isStarted = true;
}


//-----------------------------------------------------------------------------------------------
// Processes Input for all PlayModes
//
void PlayMode::ProcessInput()
{
	ProcessCursorInput();
}


//-----------------------------------------------------------------------------------------------
// Base Update for all PlayModes, moves camera onto the cursor
//
void PlayMode::Update()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the cursor in the world
//
void PlayMode::RenderWorldSpace() const
{
	// Render the cursor
	Renderer* renderer = Renderer::GetInstance();
	renderer->BindTexture(0, "White");
	RenderSingleTile(m_cursorCoords, Rgba(0,0,255, 200));
}


//-----------------------------------------------------------------------------------------------
// Renders the hover actor's data to screen
//
void PlayMode::RenderScreenSpace() const
{
	RenderHoverData();
	RenderTurnOrderWidget();
}


//-----------------------------------------------------------------------------------------------
// Called before deletion
//
void PlayMode::Exit()
{
}


//-----------------------------------------------------------------------------------------------
// Returns whether this mode has been started
//
bool PlayMode::IsStarted() const
{
	return m_isStarted;
}


//-----------------------------------------------------------------------------------------------
// Returns whether this mode has finished its task and is pending deletion
//
bool PlayMode::IsFinished() const
{
	return m_isFinished;
}


//-----------------------------------------------------------------------------------------------
// Checks for input to move the cursor and moves it
//
void PlayMode::ProcessCursorInput()
{
	// Get the input direction
	XboxController& controller = InputSystem::GetPlayerOneController();

	if (!controller.WasStickJustPressed(XBOX_STICK_LEFT))
	{
		return;
	}

	IntVector2 inputDirection = IntVector2(controller.GetCardinalStickDirection(XBOX_STICK_LEFT));

	// Get the map "up" relative to the camera
	IntVector2 cameraRelativeUp = Game::GetCurrentBoardState()->GetMapUpRelativeToCamera();

	// Determine the direction to move give the relative up and input direction
	IntVector2 finalCursorChange;

	if		(cameraRelativeUp == IntVector2::STEP_NORTH)	{ finalCursorChange = inputDirection; }
	else if (cameraRelativeUp == IntVector2::STEP_SOUTH)	{ finalCursorChange = -1 * inputDirection; }
	else if (cameraRelativeUp == IntVector2::STEP_EAST)		{ finalCursorChange = IntVector2(inputDirection.y, -1 * inputDirection.x); }
	else													{ finalCursorChange = IntVector2(-1 * inputDirection.y, inputDirection.x);}

	// Now apply the change to the cursor, clamping to the edge of the map
	Map* map = Game::GetCurrentBoardState()->GetMap();

	m_cursorCoords.x += finalCursorChange.x;
	m_cursorCoords.x = ClampInt(m_cursorCoords.x, 0, map->GetMapWidth() - 1);

	m_cursorCoords.y += finalCursorChange.y;
	m_cursorCoords.y = ClampInt(m_cursorCoords.y, 0, map->GetMapDepth() - 1);

	m_cursorCoords.z = map->GetHeightAtMapCoords(IntVector2(m_cursorCoords.x, m_cursorCoords.y));

	// Update the actor we're currently hovering over
	m_hoverActor = Game::GetCurrentBoardState()->GetActorAtCoords(m_cursorCoords.xy());
}


// -----------------------------------------------------------------------------------------------
// Renders the actor data of the hovered actor, if there is one
// 
void PlayMode::RenderHoverData() const
{
	std::string text = "No Actor Selected";
	if (m_hoverActor != nullptr)
	{
		// Name
		std::string actorName = m_hoverActor->GetActorName();

		// Class
		std::string className = m_hoverActor->GetDefinitionName();

		// Team
		int team = m_hoverActor->GetTeamIndex();

		// Health
		int health = m_hoverActor->GetCurrentHealth();

		// Wait time
		float waitTime = m_hoverActor->GetWaitTime();

		// Turn order
		BoardState* boardState = Game::GetCurrentBoardState();
		int turnOrder = boardState->GetActorTurnOrder(m_hoverActor);

		text = Stringf("Name: %s\nClass: %s\nTeam: %i\nHealth: %i\nWait Time: %.2f\nTurn Order: %i", actorName.c_str(), className.c_str(), team, health, waitTime, turnOrder + 1);
	}

	// Render
	Renderer* renderer = Renderer::GetInstance();

	renderer->BindTexture(0, "White");
	renderer->DrawAABB2(s_hoverBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0,0,0,200));

	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	renderer->DrawTextInBox2D(text, s_hoverBounds, Vector2::ZERO, 50.f, TEXT_DRAW_SHRINK_TO_FIT, font);
}


// -----------------------------------------------------------------------------------------------
// Renders the order of the next actors, based on their wait time and action speed
// 
void PlayMode::RenderTurnOrderWidget() const
{
	BoardState* boardState = Game::GetCurrentBoardState();
	
	// Always show at least 8
	std::string widgetText = "TURN ORDER\n";
	for (int actorIndex = 0; actorIndex < (int) boardState->GetActorCount(); ++actorIndex)
	{
		Actor* actor = boardState->GetActorByIndex(actorIndex);

		if (actor != nullptr)
		{
			widgetText += std::to_string(actorIndex + 1) + ". " + actor->GetActorName() + "\n";
		}
	}

	Renderer* renderer = Renderer::GetInstance();
	renderer->BindTexture(0, "White");
	renderer->DrawAABB2(s_turnOrderBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0,0,0,200));

	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	renderer->DrawTextInBox2D(widgetText, s_turnOrderBounds, Vector2::ZERO, 50.f, TEXT_DRAW_SHRINK_TO_FIT, font);
}


// -----------------------------------------------------------------------------------------------
// Draws a single tile at the given coordinates (coordinates should be an air block, tile is rendered
// at the bottom)
// 
void PlayMode::RenderSingleTile(const IntVector3& tileCoords, const Rgba& color) const
{
	Renderer* renderer = Renderer::GetInstance();
	Map* map = Game::GetCurrentBoardState()->GetMap();

	Vector3 tilePos = map->MapCoordsToWorldPosition(tileCoords);

	Vector3 mins = tilePos - Vector3(0.5f, -0.01f, 0.5f);
	Vector3 maxs = tilePos + Vector3(0.5f, 0.01f, 0.5f);

	Vertex3D_PCU vertices[6];
	vertices[0].m_position = mins;
	vertices[0].m_color = color;
	vertices[1].m_position = Vector3(maxs.x, tilePos.y + 0.01f, mins.z);
	vertices[1].m_color = color;
	vertices[2].m_position = maxs;
	vertices[2].m_color = color;
	vertices[3] = vertices[0];
	vertices[4] = vertices[2];
	vertices[5].m_position = Vector3(mins.x, tilePos.y + 0.01f, maxs.z);
	vertices[5].m_color = color;

	renderer->DrawMeshImmediate(vertices, 6, PRIMITIVE_TRIANGLES);
}
