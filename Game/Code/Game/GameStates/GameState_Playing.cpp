/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Entity/NPCTank.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Entity/Swarmer.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Entity/TankSpawner.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Entity/SwarmerSpawner.hpp"
#include "Game/GameStates/GameState_Playing.hpp"

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

#include "Engine/Core/Time/ScopedProfiler.hpp"


//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
	AABB2 uiBounds = Renderer::GetUIBounds();
	Vector2 center = uiBounds.GetCenter();
	m_crosshairBounds = AABB2(center - Vector2(25.f), center + Vector2(25.f));

	m_reloadTimerBounds = AABB2(m_crosshairBounds.GetBottomRight(), m_crosshairBounds.GetBottomRight() + Vector2(200.f, 50.f));
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	Game::DeleteMap();
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	Swarmer::InitializeConsoleCommands();

	// Make the player
	Game::InitializePlayer();

	// Make the map
	Game::InitializeMap(AABB2(Vector2(-500.f, -500.f), Vector2(500.f, 500.f)), 0.f, 25.f, IntVector2(16, 16), "Data/Images/Map.jpg");

	Camera* playerCamera = Game::GetPlayer()->GetCamera();
	Game::GetRenderScene()->AddCamera(playerCamera);
	Light* directionalLight = Light::CreateDirectionalLight(Vector3(10.f, 50.f, 10.f), Vector3(-1.f, -1.f, 0.f), Rgba(200, 200, 200, 255));
	directionalLight->SetShadowCasting(true);

	Game::GetRenderScene()->AddLight(directionalLight);
	Game::GetRenderScene()->SetAmbience(Rgba(255, 255, 255, 50));
 
	// Test the debug render system
 	DebugRenderSystem::SetWorldCamera(playerCamera);

	Mouse& mouse = InputSystem::GetMouse();

	mouse.SetCursorMode(CURSORMODE_RELATIVE);
	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);

	// Testing
// 	TankSpawner* spawner = new TankSpawner(Vector3(10.f, 15.f, 10.f), 1);
// 	Game::GetMap()->AddGameEntity(spawner);

	SwarmerSpawner* spawner2 = new SwarmerSpawner(Vector3(10.f, 15.f, 10.f), 1);
	Game::GetMap()->AddGameEntity(spawner2);

	SwarmerSpawner* spawner3 = new SwarmerSpawner(Vector3(-70.f, 15.f, 50.f), 1);
	Game::GetMap()->AddGameEntity(spawner3);

	SwarmerSpawner* spawner4 = new SwarmerSpawner(Vector3(100.f, 15.f, -50.f), 1);
	Game::GetMap()->AddGameEntity(spawner4);

	SwarmerSpawner* spawner5 = new SwarmerSpawner(Vector3(-30.f, 15.f, -120.f), 1);
	Game::GetMap()->AddGameEntity(spawner5);
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Renders the screenspace the elements of the game
//
void GameState_Playing::RenderUI() const
{
	Renderer* renderer = Renderer::GetInstance();

	renderer->SetCurrentCamera(renderer->GetUICamera());

	renderer->Draw2DQuad(m_crosshairBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLUE, AssetDB::GetSharedMaterial("Data/Materials/Crosshair.material"));

	float playerReloadTime = Game::GetPlayer()->GetTimeUntilNextShot();

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
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	Player* player = Game::GetPlayer();
	player->ProcessInput();

	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed('I'))
	{
		Light* light = Light::CreatePointLight(player->transform.position, Rgba::WHITE, Vector3(0.f, 0.f, 0.001f));
		Game::GetRenderScene()->AddLight(light);
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_SHIFT))
	{
		AudioSystem* audio = AudioSystem::GetInstance();

		if (!m_songPlaying)
		{
			m_song = audio->PlaySound(audio->CreateOrGetSound("Data/Sound/Music/Song.mp3"));
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


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{	
	Game::GetMap()->Update();
}


//-----------------------------------------------------------------------------------------------
// Renders the gameplay state to screen
//
void GameState_Playing::Render() const
{
	ForwardRenderingPath::Render(Game::GetRenderScene());
	RenderUI();
}
