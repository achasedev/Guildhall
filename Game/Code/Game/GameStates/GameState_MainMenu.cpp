/************************************************************************/
/* File: GameState_MainMenu.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_MainMenu class
/************************************************************************/
#include "Game/Framework/App.hpp"
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/Menu.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/VoxelGrid.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelEmitter.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Input/InputSystem.hpp"


//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: GameState(0.f, 0.f)
{
	MoveToSubMenu(SUB_MENU_MAIN);

	m_emitters[0] = new VoxelEmitter(50.f, 2.0f, Vector3(64.f, 8.f, 160.f), Vector3(0.f, 100.f, 0.f), 20.f);
	m_emitters[1] = new VoxelEmitter(50.f, 2.0f, Vector3(192.f, 8.f, 160.f), Vector3(0.f, 100.f, 0.f), 20.f);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameState_MainMenu::~GameState_MainMenu()
{
	delete m_emitters[0];
	m_emitters[0] = nullptr;

	delete m_emitters[1];
	m_emitters[1] = nullptr;

	if (m_currentMenu != nullptr)
	{
		delete m_currentMenu;
		m_currentMenu = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input related to this GameState and changes state accordingly
//
void GameState_MainMenu::ProcessInput()
{
	m_currentMenu->ProcessInput();

	InputSystem* input = InputSystem::GetInstance();

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
		//Game::GetGameCamera()->LookAt(m_defaultCameraPosition, Vector3(128.f, 32.f, 128.f));
		Game::GetGameCamera()->LookAtGridCenter();
	}

	m_emitters[0]->Update();
	m_emitters[1]->Update();

	Game::GetWorld()->Update();
}


//-----------------------------------------------------------------------------------------------
// Renders the Main Menu
//
void GameState_MainMenu::Render() const
{
	Game::GetWorld()->DrawToGrid();

	DrawCurrentMenu();

	VoxelFont* menuFont = Game::GetMenuFont();

	VoxelFontDraw_t options;
	options.mode = VOXEL_FONT_FILL_NONE;
	options.textColor = Rgba::BLUE;
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.scale = IntVector3(1, 1, 1);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.5f, 0.5f);
	options.borderThickness = 0;

	Game::GetVoxelGrid()->DrawVoxelText("BACKGROUND TEXT", IntVector3(128, 8, 255), options);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state
//
void GameState_MainMenu::Render_Leave() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first running update
//
bool GameState_MainMenu::Enter()
{
	Game::PlayBGM("Data/Music/Song.wav");

	return true;
}


//-----------------------------------------------------------------------------------------------
// Called when the game transitions out of this state, before deletion
//
bool GameState_MainMenu::Leave()
{
	Game::GetWorld()->CleanUp();
	return true;
}


//-----------------------------------------------------------------------------------------------
// Renders the enter state
//
void GameState_MainMenu::Render_Enter() const
{
	Render();
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for changing the sub menu of the main menu
//
void LoadSubMenu(GameState_MainMenu* mainMenu, const std::string& args)
{
	if (args == "Main")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_MAIN);
	}
	else if (args == "Leaderboard_1")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_LEADERBOARD_1);
	}
	else if (args == "Leaderboard_2")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_LEADERBOARD_2);
	}
	else if (args == "Leaderboard_3")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_LEADERBOARD_3);
	}
	else if (args == "Leaderboard_4")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_LEADERBOARD_4);
	}
	else if (args == "Episodes")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_EPISODES);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for starting an episode campaign from the main menu
//
void StartEpisode(GameState_MainMenu* mainMenu, const std::string& args)
{
	Game::TransitionToGameState(new GameState_Playing());
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for quitting the game with UI option
//
void QuitSelection(GameState_MainMenu* mainMenu, const std::string& args)
{
	App::GetInstance()->Quit();
}


//-----------------------------------------------------------------------------------------------
// Updates the menu to list the options given by the subMenu
//
void GameState_MainMenu::MoveToSubMenu(eSubMenu subMenu)
{
	if (m_currentMenu != nullptr)
	{
		ParticalizeCurrentMenu();
		delete m_currentMenu;
	}

	m_currentMenu = new Menu(this);

	switch (subMenu)
	{
	case SUB_MENU_MAIN:
		m_currentMenu->AddOption("Play",		true, LoadSubMenu, "Episodes");
		m_currentMenu->AddOption("Leaderboard", true, LoadSubMenu, "Leaderboard_1");
		m_currentMenu->AddOption("Quit",		true, QuitSelection, "");
		break;
	case SUB_MENU_EPISODES:
		m_currentMenu->AddOption("Episode 1",	true, StartEpisode, "Episode 1");
		m_currentMenu->AddOption("Episode 2",	true, StartEpisode, "Episode 2");
		m_currentMenu->AddOption("Episode 3",	true, StartEpisode, "Episode 3");
		m_currentMenu->AddOption("Episode 4",	true, StartEpisode, "Episode 4");
		m_currentMenu->AddOption("Episode 5",	true, StartEpisode, "Episode 5");
		m_currentMenu->AddOption("Back",		true, LoadSubMenu, "Main");
		break;
	case SUB_MENU_LEADERBOARD_1:
	{
		const Leaderboard* boards = Game::GetLeaderboards();
		m_currentMenu->AddOption(boards[0].m_name, false, nullptr, "");

		for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
		{
			m_currentMenu->AddOption(Stringf("%i", boards[0].m_scores[i]), false, nullptr, "");
		}

		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");

		m_currentMenu->SetLeftOption(LoadSubMenu, "Leaderboard_4");
		m_currentMenu->SetRightOption(LoadSubMenu, "Leaderboard_2");

		m_currentMenu->SetCursorPosition(6); // Set the cursor on the only selectable option
	}
		break;
	case SUB_MENU_LEADERBOARD_2:
	{
		const Leaderboard* boards = Game::GetLeaderboards();
		m_currentMenu->AddOption(boards[1].m_name, false, nullptr, "");

		for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
		{
			m_currentMenu->AddOption(Stringf("%i", boards[1].m_scores[i]), false, nullptr, "");
		}

		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");

		m_currentMenu->SetLeftOption(LoadSubMenu, "Leaderboard_1");
		m_currentMenu->SetRightOption(LoadSubMenu, "Leaderboard_3");

		m_currentMenu->SetCursorPosition(6); // Set the cursor on the only selectable option
	}
		break;
	case SUB_MENU_LEADERBOARD_3:
	{
		const Leaderboard* boards = Game::GetLeaderboards();
		m_currentMenu->AddOption(boards[2].m_name, false, nullptr, "");

		for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
		{
			m_currentMenu->AddOption(Stringf("%i", boards[2].m_scores[i]), false, nullptr, "");
		}

		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");

		m_currentMenu->SetLeftOption(LoadSubMenu, "Leaderboard_2");
		m_currentMenu->SetRightOption(LoadSubMenu, "Leaderboard_4");

		m_currentMenu->SetCursorPosition(6); // Set the cursor on the only selectable option
	}
		break;
	case SUB_MENU_LEADERBOARD_4:
	{
		const Leaderboard* boards = Game::GetLeaderboards();
		m_currentMenu->AddOption(boards[3].m_name, false, nullptr, "");

		for (int i = 0; i < NUM_SCORES_PER_LEADERBOARD; ++i)
		{
			m_currentMenu->AddOption(Stringf("%i", boards[3].m_scores[i]), false, nullptr, "");
		}

		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");

		m_currentMenu->SetLeftOption(LoadSubMenu, "Leaderboard_3");
		m_currentMenu->SetRightOption(LoadSubMenu, "Leaderboard_1");

		m_currentMenu->SetCursorPosition(6); // Set the cursor on the only selectable option
	}
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the current menu to the grid
//
void GameState_MainMenu::DrawCurrentMenu() const
{
	IntVector3 drawPosition = m_menuStartCoord;
	VoxelFont* menuFont = Game::GetMenuFont();
	int cursorPosition = m_currentMenu->GetCursorPosition();

	std::vector<std::string> texts = m_currentMenu->GetTextsForRender();
	for (int textIndex = 0; textIndex < (int)texts.size(); ++textIndex)
	{
		Rgba color = Rgba::WHITE;
		if (textIndex == cursorPosition)
		{
			color = Rgba::YELLOW;
		}

		VoxelFontDraw_t options;
		options.mode = VOXEL_FONT_FILL_NONE;
		options.textColor = color;
		options.fillColor = Rgba::BLUE;
		options.font = menuFont;
		options.scale = IntVector3(1, 1, 1);
		options.up = IntVector3(0, 0, 1);
		options.alignment = Vector3(0.5f, 0.5f, 0.5f);
		options.borderThickness = 0;

		Game::GetVoxelGrid()->DrawVoxelText(texts[textIndex], drawPosition, options);

		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}
}


//-----------------------------------------------------------------------------------------------
// Particalizes the current menu into the world
//
void GameState_MainMenu::ParticalizeCurrentMenu() const
{
	IntVector3 drawPosition = m_menuStartCoord;
	VoxelFont* menuFont = Game::GetMenuFont();
	int cursorPosition = m_currentMenu->GetCursorPosition();

	std::vector<std::string> texts = m_currentMenu->GetTextsForRender();
	for (int textIndex = 0; textIndex < (int)texts.size(); ++textIndex)
	{
		Rgba color = Rgba::WHITE;
		if (textIndex == cursorPosition)
		{
			color = Rgba::YELLOW;
		}

		VoxelFontDraw_t options;
		options.mode = VOXEL_FONT_FILL_NONE;
		options.textColor = color;
		options.fillColor = Rgba::BLUE;
		options.font = menuFont;
		options.scale = IntVector3(1, 1, 1);
		options.up = IntVector3(0, 0, 1);
		options.alignment = Vector3(0.5f, 0.5f, 0.5f);
		options.borderThickness = 0;

		Game::GetWorld()->ParticalizeVoxelText(texts[textIndex], drawPosition, options);

		drawPosition -= IntVector3(0, 0, 1) * (menuFont->GetGlyphDimensions().y + 5);
	}
}
