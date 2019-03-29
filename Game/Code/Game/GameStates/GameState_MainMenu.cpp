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
#include "Game/Framework/CampaignDefinition.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Default constructor
//
GameState_MainMenu::GameState_MainMenu()
	: GameState(0.f, 0.f)
{
	MoveToSubMenu(SUB_MENU_MAIN);

	m_emitters[0] = new VoxelEmitter(100.f, 2.0f, Vector3(28.f, 24.f, 254.f), Vector3(0.f, 100.f, 0.f), 20.f);
	m_emitters[1] = new VoxelEmitter(100.f, 2.0f, Vector3(225.f, 24.f, 254.f), Vector3(0.f, 100.f, 0.f), 20.f);
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
}


//-----------------------------------------------------------------------------------------------
// Updates the MainMenu state
//
void GameState_MainMenu::Update()
{
	m_emitters[0]->Update();
	m_emitters[1]->Update();

	Game::GetWorld()->Update();

	// Also Update the music from the intro - hack
	if (AudioSystem::GetInstance()->IsSoundFinished(Game::s_instance->m_trackTendingToTarget))
	{
		Game::PlayBGM("Data/Audio/Music/Theme Song 8-bit V1 _looping.wav", false);
	}
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
	options.glyphColors.push_back(Rgba::BLUE);
	options.fillColor = Rgba::BLUE;
	options.font = menuFont;
	options.scale = IntVector3(2, 2, 2);
	options.up = IntVector3(0, 1, 0);
	options.alignment = Vector3(0.5f, 0.f, 0.5f);
	options.borderThickness = 0;

	options.colorFunction = GetColorForWaveEffect;

	VoxelFontColorWaveArgs_t colorArgs;
	colorArgs.direction = IntVector3(1, 0, 0);
	colorArgs.speed = 1.0f;

	options.colorFunctionArgs = &colorArgs;
	options.offsetFunction = GetOffsetForFontWaveEffect;

	Game::GetVoxelGrid()->DrawVoxelText("VOXEL HEROES", IntVector3(128, 24, 255), options);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state
//
void GameState_MainMenu::Render_Leave() const
{
	Render();
}

#include "Game/Entity/AIEntity.hpp"
#include "Game/Entity/Components/BehaviorComponent_Wander.hpp"
#include "Game/Framework/MapDefinition.hpp"

//-----------------------------------------------------------------------------------------------
// Called when the game transitions into this state, before the first running update
//
bool GameState_MainMenu::Enter()
{
	Game::SetBGMVolume(1.0f);

	AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/Theme Song 8-bit V1 _looping.wav");
	Game::PlayBGM("Data/Audio/Music/Theme Song 8-bit V1 _opening.wav", true, false);

	World* world = Game::GetWorld();
	const MapDefinition* mapDef = MapDefinition::GetRandomDefinition();
	world->IntializeMap(mapDef);

	std::vector<const EntityDefinition*> playerDefs = EntityDefinition::GetAllPlayerDefinitions();

	int numPlayers = (int)playerDefs.size();
	for (int playerIndex = 0; playerIndex < numPlayers; ++playerIndex)
	{
		AIEntity* newEntity = new AIEntity(playerDefs[playerIndex]);

		newEntity->SetBehaviorComponent(new BehaviorComponent_Wander());

		newEntity->SetPosition(Vector3(GetRandomFloatInRange(0.f, 240.f), 64.f, GetRandomFloatInRange(0.f, 240.f)));
		newEntity->SetOrientation(GetRandomFloatInRange(0.f, 360.f));

		world->AddEntity(newEntity);
	}

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
	std::vector<std::string> tokens = Tokenize(args, ' ');
	std::string& subMenuName = tokens[0];

	if (subMenuName == "Main")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_MAIN);
	}
	else if (subMenuName == "Leaderboard")
	{
		int playerCountIndex = StringToInt(tokens[1]);
		int leaderboardIndex = StringToInt(tokens[2]);
		mainMenu->SetLeaderboardDisplayParameters(playerCountIndex, leaderboardIndex);

		mainMenu->MoveToSubMenu(SUB_MENU_LEADERBOARD);
	}
	else if (subMenuName == "Episodes")
	{
		mainMenu->MoveToSubMenu(SUB_MENU_CAMPAIGNS);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for starting an episode campaign from the main menu
//
void StartCampaign(GameState_MainMenu* mainMenu, const std::string& campaignName)
{
	UNUSED(mainMenu);

	Game::TransitionToGameState(new GameState_Playing(campaignName));
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Callback for quitting the game with UI option
//
void QuitSelection(GameState_MainMenu* mainMenu, const std::string& args)
{
	UNUSED(args);
	UNUSED(mainMenu);

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
		m_currentMenu->AddOption("Leaderboard", true, LoadSubMenu, Stringf("Leaderboard %i %i", m_playerCountScoreboardIndexBeingDisplayed, m_leaderboardIndexBeingDisplayed));
		m_currentMenu->AddOption("Quit",		true, QuitSelection, "");
		break;
	case SUB_MENU_CAMPAIGNS:
	{
		// Push every campaign we have
		std::map<std::string, const CampaignDefinition*>::const_iterator itr = CampaignDefinition::s_campaignDefinitions.begin();

		for (itr; itr != CampaignDefinition::s_campaignDefinitions.end(); itr++)
		{
			const CampaignDefinition* def = itr->second;
			m_currentMenu->AddOption(def->m_name, true, StartCampaign, def->m_name);
		}
		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");
		break;
	}
	case SUB_MENU_LEADERBOARD:
	{
		const Leaderboard& leaderboard = Game::GetLeaderboardByIndex(m_leaderboardIndexBeingDisplayed);
		const ScoreBoard& scoreboard = leaderboard.m_scoreboards[m_playerCountScoreboardIndexBeingDisplayed];

		m_currentMenu->AddOption(leaderboard.m_name, false, nullptr, "");
		m_currentMenu->AddOption(scoreboard.m_name, false, nullptr, "");

		for (int i = 0; i < NUM_SCORES_PER_SCOREBOARD; ++i)
		{
			m_currentMenu->AddOption(Stringf("%i", scoreboard.m_scores[i]), false, nullptr, "");
		}

		m_currentMenu->AddOption("Back", true, LoadSubMenu, "Main");

		int leftPlayerCountIndex = (m_playerCountScoreboardIndexBeingDisplayed - 1 < 0 ? MAX_PLAYERS - 1 : m_playerCountScoreboardIndexBeingDisplayed - 1);
		int rightPlayerCountIndex = (m_playerCountScoreboardIndexBeingDisplayed + 1 >= MAX_PLAYERS ? 0 : m_playerCountScoreboardIndexBeingDisplayed + 1);
		
		int numCampaigns = CampaignDefinition::GetCampaignCount();
		int prevLeaderboardIndex = (m_leaderboardIndexBeingDisplayed - 1 < 0 ? numCampaigns - 1 : m_leaderboardIndexBeingDisplayed - 1);
		int nextLeaderboardIndex = (m_leaderboardIndexBeingDisplayed + 1 >= numCampaigns ? 0 : m_leaderboardIndexBeingDisplayed + 1);

		m_currentMenu->SetLeftOption(LoadSubMenu, Stringf("Leaderboard %i %i", leftPlayerCountIndex, m_leaderboardIndexBeingDisplayed));
		m_currentMenu->SetRightOption(LoadSubMenu, Stringf("Leaderboard %i %i", rightPlayerCountIndex, m_leaderboardIndexBeingDisplayed));
		m_currentMenu->SetUpOption(LoadSubMenu, Stringf("Leaderboard %i %i", m_playerCountScoreboardIndexBeingDisplayed, prevLeaderboardIndex));
		m_currentMenu->SetDownOption(LoadSubMenu, Stringf("Leaderboard %i %i", m_playerCountScoreboardIndexBeingDisplayed, nextLeaderboardIndex));

		m_currentMenu->SetCursorPosition(7); // Set the cursor on the only selectable option
	}
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the display parameters for showing leaderboards on the main menu
//
void GameState_MainMenu::SetLeaderboardDisplayParameters(int playerCountIndex, int leaderboardIndex)
{
	m_playerCountScoreboardIndexBeingDisplayed = playerCountIndex;
	m_leaderboardIndexBeingDisplayed = leaderboardIndex;
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
		options.glyphColors.push_back(color);
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
		options.glyphColors.push_back(color);
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
