/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Entity/Cannon.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Swarmer.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/TankSpawner.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Entity/SwarmerSpawner.hpp"
#include "Game/GameStates/GameState_Playing.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Resources/Sampler.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Particles/ParticleEmitter.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Core/Time/Clock.hpp"

#include "Engine/Core/Time/ProfileScoped.hpp"

const float GameState_Playing::RESPAWN_WAIT_TIME = 5.f;



//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
	AABB2 uiBounds = Renderer::GetUIBounds();
	Vector2 center = uiBounds.GetCenter();
	m_crosshairBounds = AABB2(center - Vector2(25.f), center + Vector2(25.f));

	m_reloadTimerBounds = AABB2(m_crosshairBounds.GetBottomRight(), m_crosshairBounds.GetBottomRight() + Vector2(200.f, 50.f));

	m_chargeTimerbounds = AABB2(m_crosshairBounds.GetBottomLeft() - Vector2(0.f, 15.f), m_crosshairBounds.GetBottomRight() - Vector2(0.f, 10.f));
	m_respawnTimer = new Stopwatch(Clock::GetMasterClock());
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	delete m_respawnTimer;
	m_respawnTimer = nullptr;

	Game::GetMap()->KillAllEnemies();
	Game::GetMap()->DeleteObjectsMarkedForDelete();

	DebugRenderSystem::SetWorldCamera(nullptr);
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	//-----Input-----
	Mouse& mouse = InputSystem::GetMouse();
	mouse.SetCursorMode(CURSORMODE_RELATIVE);
	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);

	// Spawn entities
	SpawnInitialEntities();
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}



void GameState_Playing::SpawnInitialEntities()
{
	Map* map = Game::GetMap();

	// 	TankSpawner* spawner = new TankSpawner(Vector3(10.f, 15.f, 10.f), 1);
	// 	Game::GetMap()->AddGameEntity(spawner);

	SwarmerSpawner* spawner2 = new SwarmerSpawner(Vector3(10.f, 25.f, 10.f), 1);
	map->AddGameEntity(spawner2);

	SwarmerSpawner* spawner3 = new SwarmerSpawner(Vector3(-70.f, 25.f, 50.f), 1);
	map->AddGameEntity(spawner3);

	SwarmerSpawner* spawner4 = new SwarmerSpawner(Vector3(100.f, 25.f, -50.f), 1);
	map->AddGameEntity(spawner4);

	SwarmerSpawner* spawner5 = new SwarmerSpawner(Vector3(-30.f, 25.f, -120.f), 1);
	map->AddGameEntity(spawner5);
}

//-----------------------------------------------------------------------------------------------
// Checks if any enemies are left alive on the map, and if so transitions to the victory state
//
bool GameState_Playing::CheckForVictory() const
{
	Map* map = Game::GetMap();
	Player* player = Game::GetPlayer();

	std::vector<GameEntity*>& entities = map->GetEntitiesOnMap();

	unsigned int playerTeam = player->GetTeamIndex();

	for (int entityIndex = 0; entityIndex < (int) entities.size(); ++entityIndex)
	{
		if (entities[entityIndex]->GetTeamIndex() != playerTeam)
		{
			return false;
		}
	}

	// No enemies found, so you won!
	return true;
}


//-----------------------------------------------------------------------------------------------
// Renders the screenspace the elements of the game
//
void GameState_Playing::RenderUI() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());

	RenderUI_Playing();

	if (m_playerWon)
	{
		RenderUI_Victory();
	}
	else if (m_playerDead)
	{
		RenderUI_Death();
	}
}


void GameState_Playing::RenderUI_Playing() const
{
	Renderer* renderer = Renderer::GetInstance();
	Player* player = Game::GetPlayer();
	Map* map = Game::GetMap();

	renderer->Draw2DQuad(m_crosshairBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLUE, AssetDB::GetSharedMaterial("Data/Materials/Crosshair.material"));

	float playerReloadTime = player->GetTimeUntilNextShot();

	std::string reloadText;
	Rgba textColor;
	if (playerReloadTime > 0.f)
	{
		reloadText = Stringf("%.2f%s", playerReloadTime, "s");
		textColor = Rgba::RED;
	}
	else
	{
		reloadText = "Ready";
		textColor = Rgba::GREEN;
	}

	renderer->DrawTextInBox2D(reloadText, m_reloadTimerBounds, Vector2(0.2f, 0.5f), 20.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"), textColor);
	renderer->DrawTextInBox2D(Stringf("Health: %i", player->GetHealth()), Renderer::GetUIBounds(), Vector2(0.f, 0.f), 50.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"));
	renderer->DrawTextInBox2D(Stringf("Enemy Count: %i", map->GetEnemyCount()), Renderer::GetUIBounds(), Vector2(1.f, 0.f), 50.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"));

	float chargeFill = ClampFloatZeroToOne(player->GetChargeTimerNormalized());

	renderer->Draw2DQuad(m_chargeTimerbounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLACK, AssetDB::GetSharedMaterial("UI"));

	Rgba fillColor = (chargeFill == 1.0f ? Rgba::GREEN : Rgba::RED);

	AABB2 fillBounds = AABB2(m_chargeTimerbounds.GetBottomLeft(), m_chargeTimerbounds.GetBottomLeft() + Vector2(m_chargeTimerbounds.GetDimensions().x * chargeFill, m_chargeTimerbounds.GetDimensions().y));
	renderer->Draw2DQuad(fillBounds, AABB2::UNIT_SQUARE_OFFCENTER, fillColor, AssetDB::GetSharedMaterial("UI"));
}


//-----------------------------------------------------------------------------------------------
// Renders the victory screen
//
void GameState_Playing::RenderUI_Victory() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->Draw2DQuad(Renderer::GetUIBounds(), AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0, 0, 255, 100), AssetDB::GetSharedMaterial("UI"));

	renderer->DrawTextInBox2D("Victory!\nPress 'space' to return to the main menu", Renderer::GetUIBounds(), Vector2(0.5f), 50.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"));
}


//-----------------------------------------------------------------------------------------------
// Displays the death screen, with the respawn timer
//
void GameState_Playing::RenderUI_Death() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->Draw2DQuad(Renderer::GetUIBounds(), AABB2::UNIT_SQUARE_OFFCENTER, Rgba(255, 0, 0, 100), AssetDB::GetSharedMaterial("UI"));

	renderer->DrawTextInBox2D("YOU DIED", Renderer::GetUIBounds(), Vector2(0.5f), 100.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"));

	std::string respawnText;
	if (m_respawnTimer->HasIntervalElapsed())
	{
		respawnText = "Press 'space' to respawn";
	}
	else
	{
		respawnText = Stringf("Respawn in %.2f seconds...", m_respawnTimer->GetTimeUntilIntervalEnds());
	}

	renderer->DrawTextInBox2D(respawnText, Renderer::GetUIBounds(), Vector2(0.5f, 1.0f), 50.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/Default.png"));
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();
	Player* player = Game::GetPlayer();

	if (m_playerWon)
	{
		if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
		{
			Game::TransitionToGameState(new GameState_MainMenu());
		}
	}
	else if (m_playerDead)
	{
		if (m_respawnTimer->HasIntervalElapsed() && input->WasKeyJustPressed(InputSystem::KEYBOARD_SPACEBAR))
		{
			player->Respawn();
			m_playerDead = false;
		}
	}
	else
	{
		player->ProcessInput();

		if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SHIFT))
		{
			AudioSystem* audio = AudioSystem::GetInstance();

			if (!m_songPlaying)
			{
				m_song = audio->PlaySound(audio->CreateOrGetSound("Data/Audio/Music/Song.mp3"));
				m_songPlaying = true;
			}
			else
			{
				audio->SetSoundPlaybackSpeed(m_song, 1.0f);
			}
		}

		if (input->WasKeyJustReleased(InputSystem::KEYBOARD_SHIFT))
		{
			AudioSystem* audio = AudioSystem::GetInstance();
			audio->SetSoundPlaybackSpeed(m_song, 0.f);
		}
	}
}

#include "Engine/Core/Time/ProfileLogScoped.hpp"

//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{	
	PROFILE_LOG_SCOPE("GameState_Playing:Update");

	Player* player = Game::GetPlayer();

	m_playerWon = CheckForVictory();
	bool playerJustDied = player->IsMarkedForDelete();

	if (!m_playerDead && playerJustDied)
	{
		m_playerDead = true;
		m_respawnTimer->SetInterval(RESPAWN_WAIT_TIME);
	}

	Map* map = Game::GetMap();
	map->Update();
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());

	// Render the laser sight
	Player* player = Game::GetPlayer();
	Transform muzzle = player->GetMuzzleTransform();

	// Also show where the gun is aiming
	Vector3 position = muzzle.GetParentsToWorldMatrix().TransformPoint(muzzle.position).xyz();
	Vector3 direction = muzzle.GetWorldForward();
	RaycastHit_t hit = Game::GetMap()->Raycast(position, direction, Map::MAX_RAYCAST_DISTANCE);

	// Don't actually care if it hit, just render a debug to the end position
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawLine(position, Rgba::RED, hit.position, Rgba::WHITE);
	renderer->DrawSphere(hit.position, 0.2f, 8, 4, Rgba::WHITE);

	RenderUI();
}
