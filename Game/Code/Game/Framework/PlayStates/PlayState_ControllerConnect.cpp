/************************************************************************/
/* File: PlayState_ControllerConnect.cpp
/* Author: Andrew Chase
/* Date: January 7th 2018
/* Description: Implementation of the Controller connection PlayState
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/PlayStates/PlayState_ControllerConnect.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
PlayState_ControllerConnect::PlayState_ControllerConnect(int controllerIndex)
	: PlayState(CONTROLLER_TRANSITION_IN_TIME, CONTROLLER_TRANSITION_OUT_TIME)
	, m_controllerIndex(controllerIndex)
{
	m_menuText.push_back(Stringf("Player %i", controllerIndex + 1));
	m_menuText.push_back("Disconnected");
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
PlayState_ControllerConnect::~PlayState_ControllerConnect()
{
}


//-----------------------------------------------------------------------------------------------
// Checks for input to return to the main menu
//
void PlayState_ControllerConnect::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = input->GetController(m_controllerIndex);

	bool startPressed = controller.WasButtonJustPressed(XBOX_BUTTON_START);

	if (startPressed)
	{
		m_resumePressed = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Enter update
//
bool PlayState_ControllerConnect::Enter()
{
	// Play a sound and translate the menu in (?)
	float t = m_transitionTimer.GetElapsedTimeNormalized();
	m_menuStartPosition.y = Interpolate(0, 40, t);

	return m_transitionTimer.HasIntervalElapsed();
}


//-----------------------------------------------------------------------------------------------
// Update
//
void PlayState_ControllerConnect::Update()
{
	InputSystem* input = InputSystem::GetInstance();
	XboxController& controller = input->GetController(m_controllerIndex);

	if (controller.IsConnected() && m_menuText[1] == "Disconnected")
	{
		// Push in the "Press start" text
		m_menuText[1] = "Reconnected";
		m_menuText.push_back("Press Start");
	}
	else if (!controller.IsConnected() && m_menuText[1] == "Reconnected")
	{
		// Remove the press start text and display disconnected
		m_menuText.pop_back();
		m_menuText[1] = "Disconnected";
	}
	
	if (m_resumePressed)
	{
		m_gameState->PopOverrideState();
	}
}


//-----------------------------------------------------------------------------------------------
// Leave
//
bool PlayState_ControllerConnect::Leave()
{
	// More sound and stuff here, volume adjustment
	static bool isParticalized = false;
	if (!isParticalized)
	{
		// Particalize the menu
		VoxelFont* menuFont = Game::GetMenuFont();

		VoxelFontDraw_t options;
		options.mode = VOXEL_FONT_FILL_NONE;
		options.textColor = Rgba::BLUE;
		options.fillColor = Rgba::BLUE;
		options.font = menuFont;
		options.alignment = Vector3(0.5f, 0.5f, 0.5f);
		options.borderThickness = 0;
		options.scale = IntVector3::ONES;
		options.up = IntVector3(0, 0, 1);

		// Draw the menu options
		IntVector3 drawPosition = IntVector3(128, 40, 160);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

		for (int i = 0; i < m_menuText.size(); ++i)
		{
			Game::GetWorld()->ParticalizeVoxelText(m_menuText[i], drawPosition, options);
			drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
			if (i == 1)
			{
				drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
			}
		}

		isParticalized = true;
	}

	UpdateWorldAndCamera();

	bool finishedLeaving = m_transitionTimer.HasIntervalElapsed();

	if (finishedLeaving)
	{
		isParticalized = false;
	}

	return finishedLeaving;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter transition
//
void PlayState_ControllerConnect::Render_Enter() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Renders the running state
//
void PlayState_ControllerConnect::Render() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();

	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::BLUE;
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;
	options.scale = IntVector3::ONES;
	options.up = IntVector3(0, 0, 1);

	// Draw the menu options
	IntVector3 drawPosition = m_menuStartPosition;
	drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);

	for (int i = 0; i < m_menuText.size(); ++i)
	{
		Game::GetVoxelGrid()->DrawVoxelText(m_menuText[i], drawPosition, options);
		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
		if (i == 1)
		{
			drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the leave transition
//
void PlayState_ControllerConnect::Render_Leave() const
{
	Game::GetWorld()->DrawToGrid();
	Game::DrawPlayerHUD();
}
