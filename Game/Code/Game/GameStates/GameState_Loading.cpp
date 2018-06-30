/************************************************************************/
/* File: GameState_Loading.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_Loading class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Loading.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Resources/Skybox.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/Material.hpp"

//-----------------------------------------------------------------------------------------------
// Necessary override imposed by the GameState base class, is unused
//
void GameState_Loading::ProcessInput()
{
}


//-----------------------------------------------------------------------------------------------
// Checks if the scene is rendered, and if so loads resources from disk
//
void GameState_Loading::Update()
{
	// If the scene is already drawn, then load the resources and transition
	if (m_isSceneRendered)
	{
		LoadResources();
		Game::TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// We will hit Render() right after this, but since Render is const
		// we need to set the member true here for now
		m_isSceneRendered = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the loading screen
//
void GameState_Loading::Render() const
{
	// Draw the loading screen
	Renderer* renderer = Renderer::GetInstance();

	renderer->ClearScreen(Rgba::BLUE);
	renderer->SetCurrentCamera(renderer->GetUICamera());

	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Data/Images/Fonts/Default.png");
	AABB2 loadingBounds = AABB2(Vector2(0.35f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.3f * Renderer::UI_ORTHO_HEIGHT), Vector2(0.65f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.7f * Renderer::UI_ORTHO_HEIGHT));
	renderer->DrawTextInBox2D("Loading Game...\n", loadingBounds, Vector2(0.5f, 0.5f), 50.f, TEXT_DRAW_OVERRUN, font);
}


void GameState_Loading::Enter()
{
}


void GameState_Loading::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Loads external resources from disk using the Renderer
//
void GameState_Loading::LoadResources() const
{
	// To print the time taken
	ProfileScoped sp = ProfileScoped("Game Loading"); UNUSED(sp);

	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(Vector3::ZERO, 0.1f * Vector3::ONES, Rgba::ORANGE);
	mb.FinishBuilding();
	Mesh* mesh = mb.CreateMesh();

	AssetDB::AddMesh("Particle", mesh);

	// Set up the material for the map
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Map.material");
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Tank.material");
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Water.material");
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Crosshair.material");

	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Detail.material");
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Quad.material");
	AssetDB::CreateOrGetSharedMaterial("Data/Materials/Miku_Base.material");

	// Load all sounds
	AudioSystem* audio = AudioSystem::GetInstance();

	audio->CreateOrGetSound("Data/Audio/Music/Song.mp3");
	audio->CreateOrGetSound("Data/Audio/Music/MainMenu.mp3");
	audio->LoadAudioGroupFile("Data/Audio/AudioGroups/SoundFX.xml");

	// Load Skybox here, and set it to the scene
	Skybox* skybox = AssetDB::CreateOrGetSkybox("Data/Images/Sky.jpg");
	Game::GetRenderScene()->SetSkybox(skybox);

	// Build the map now
	// Make the map
	Map* map = new Map();
	map->Intialize(AABB2(Vector2(-500.f, -500.f), Vector2(500.f, 500.f)), 0.f, 25.f, IntVector2(16, 16), "Data/Images/Map.jpg");
	
	Game::s_instance->m_map = map;
	Game::s_instance->m_map->AddGameEntity(Game::s_instance->m_player);
}
