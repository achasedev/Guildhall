/************************************************************************/
/* File: GameState_MainMenu.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_MainMenu class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Ready.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: m_cursorPosition(0)
{
	m_menuFont = new VoxelFont("Menu", "Data/Images/Fonts/Default.png");

	m_menuOptions.push_back("Play");
	m_menuOptions.push_back("Quit");
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameState_MainMenu::~GameState_MainMenu()
{
	delete m_menuFont;
	m_menuFont = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input related to this GameState and changes state accordingly
//
void GameState_MainMenu::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	// Moving down
	bool keyPressedDown = input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW);
	if (keyPressedDown)
	{
		m_cursorPosition++;
		if (m_cursorPosition > (int) (m_menuOptions.size()) - 1)
		{
			m_cursorPosition = 0;
		}
	}

	// Moving up
	bool keyPressedUp = input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW);
	if (keyPressedUp)
	{
		m_cursorPosition--;
		if (m_cursorPosition < 0)
		{
			m_cursorPosition = (int) (m_menuOptions.size()) - 1;
		}
	}

	// Selection
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		ProcessMenuSelection();
	}

	// Quit
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_ESCAPE))
	{
		App::GetInstance()->Quit();
	}

	// Camera
	GameCamera* camera = Game::GetGameCamera();
	if (InputSystem::GetInstance()->WasKeyJustPressed('B'))
	{
		camera->ToggleEjected();
	}

	if (camera->IsEjected())
	{
		camera->UpdatePositionOnInput();
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the MainMenu state
//
void GameState_MainMenu::Update()
{
	GameCamera* camera = Game::GetGameCamera();

	if (camera != nullptr && !camera->IsEjected())
	{
		Game::GetGameCamera()->LookAt(m_defaultCameraPosition, Vector3(128.f, 32.f, 128.f));
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the Main Menu
//
void GameState_MainMenu::Render() const
{
	IntVector3 drawPosition = m_menuStartCoord;

	for (int menuIndex = 0; menuIndex < static_cast<int>(m_menuOptions.size()); ++menuIndex)
	{
		Rgba color = Rgba::WHITE;
		if (menuIndex == m_cursorPosition)
		{
			color = Rgba::YELLOW;
		}

		VoxelFontDraw_t options;
		options.mode = FILL_MODE_NONE;
		options.color = color;
		options.optionColor = Rgba::BLUE;
		options.font = m_menuFont;
		options.scale = IntVector3(1, 1, 1);
		options.up = IntVector3(0, 0, 1);
		options.alignment = Vector3(0.5f, 0.5f, 0.5f);
		options.borderThickness = 0;

		Game::GetVoxelGrid()->DrawVoxelText(m_menuOptions[menuIndex].c_str(), drawPosition, options);

		drawPosition -= IntVector3(0,0,1) * (m_menuFont->GetGlyphDimensions().y + 5);
	}

	VoxelFontDraw_t options;
	options.mode = FILL_MODE_NONE;
	options.color = Rgba::BLUE;
	options.optionColor = Rgba::BLUE;
	options.font = m_menuFont;
	options.scale = IntVector3(1, 1, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;

	Game::GetVoxelGrid()->DrawVoxelText("BACKGROUND TEXT", IntVector3(128, 8, 255), options);
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first update
//
void GameState_MainMenu::Enter()
{
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitios out of this state, before deletion
//
void GameState_MainMenu::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Processes the enter command on a menu selection
//
void GameState_MainMenu::ProcessMenuSelection() const
{
	std::string selectedOption = m_menuOptions[m_cursorPosition];

	if (selectedOption == "Play")
	{
		Game::TransitionToGameState(new GameState_Ready());
	}
	else if (selectedOption == "Quit")
	{
		App::GetInstance()->Quit();
	}
}
