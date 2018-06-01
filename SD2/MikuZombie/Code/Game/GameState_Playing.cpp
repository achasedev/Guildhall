/************************************************************************/
/* File: GameState_Playing.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_Playing class
/************************************************************************/
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/Zombie.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameState_Playing.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Light.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshGroup.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/ForwardRenderingPath.hpp"

#include "Game/Player.hpp"
#include "Game/Bullet.hpp"

#include "Engine/Renderer/Skybox.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Renderer/ParticleEmitter.hpp"
#include "Engine/Renderer/MaterialInstance.hpp"

RenderScene* GameState_Playing::s_scene = nullptr;
Player* GameState_Playing::s_Player = nullptr;

//-----------------------------------------------------------------------------------------------
// Base constructor
//
GameState_Playing::GameState_Playing()
{
}


//-----------------------------------------------------------------------------------------------
// Base destructor
//
GameState_Playing::~GameState_Playing()
{
	// Delete game objects
	for (int objIndex = 0; objIndex < (int) m_gameObjects.size(); ++objIndex)
	{
		delete m_gameObjects[objIndex];
	}

	m_gameObjects.clear();

	delete s_Player;
	s_Player = nullptr;

	delete m_playerCamera;
	m_playerCamera = nullptr;

	delete m_zombieTimer;
	m_zombieTimer = nullptr;

	delete m_fireRateTimer;
	m_fireRateTimer = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Sets up state before updating
//
void GameState_Playing::Enter()
{
	Renderer* renderer = Renderer::GetInstance();

	// Set up the game camera
	m_playerCamera = new Camera();
	m_playerCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_playerCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_playerCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);

	// Set up the player
	s_Player = new Player();
	s_Player->transform.position = Vector3(0.f, 2.f, 0.f);
	m_gameObjects.push_back(s_Player);

	m_zombieTimer = new Stopwatch(Game::GetGameClock());
	m_zombieTimer->SetTimer((1.f / m_spawnRate));
	m_fireRateTimer = new Stopwatch(Game::GetGameClock());
	m_fireRateTimer->SetTimer((1.f / PLAYER_FIRERATE));
	
	// Set up the mouse for FPS controls
	Mouse& mouse = InputSystem::GetMouse();
	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	DebugRenderSystem::SetWorldCamera(m_playerCamera);

	// Construct the render scene
	InitializeRenderScene();
}


//-----------------------------------------------------------------------------------------------
// Cleans up/finishes any operations before the game leaves this state
//
void GameState_Playing::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Returns the render scene for the playing state
//
RenderScene* GameState_Playing::GetRenderScene()
{
	return s_scene;
}


//-----------------------------------------------------------------------------------------------
// Returns the player
//
Player* GameState_Playing::GetPlayer()
{
	return s_Player;
}


//-----------------------------------------------------------------------------------------------
// Checks for input
//
void GameState_Playing::ProcessInput()
{
	float deltaTime = Game::GetDeltaTime();
	s_Player->ProcessInput(deltaTime);

	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed('N'))
	{
		AddPointLight(s_Player->transform.position + Vector3(0.f, 3.f, 0.f));
	}

	Mouse& mouse = InputSystem::GetMouse();

	if (mouse.IsButtonPressed(MOUSEBUTTON_LEFT))
	{
		SpawnBullet(s_Player);
	}

	if (input->WasKeyJustPressed('M'))
	{
		m_zombiesEnabled = !m_zombiesEnabled;
	}
}


//-----------------------------------------------------------------------------------------------
// Updates the play state
//
void GameState_Playing::Update()
{
	UpdateLights();
	UpdateGameObjects();

	CheckToSpawnZombie();
	CheckForCollisions();
	CheckToDeleteObjects();

	// Make the camera keep up with the player
	m_playerCamera->SetTransform(s_Player->transform);
	m_spawnRate += ZOMBIE_SPAWN_RATE_ACCELERATION * Game::GetDeltaTime();
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
// Sets up the render scene with all renderables and lights
//
void GameState_Playing::InitializeRenderScene()
{
	// Construct the Render Scene
	s_scene = new RenderScene();
	s_scene->AddCamera(m_playerCamera);
	s_scene->SetAmbience(Rgba(255, 255, 255, 20));

	// Add the skybox
	TextureCube* skyboxTexture = AssetDB::CreateOrGetTextureCube("Night_Sky.jpg");
	Skybox* skybox = new Skybox(skyboxTexture);
	s_scene->SetSkybox(skybox);

	// Add renderables
	BuildGeometry();
	ConstructLights();
}


//-----------------------------------------------------------------------------------------------
// Builds the static geometry for the scene
//
void GameState_Playing::BuildGeometry()
{
	//-----Ground-----
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.Push3DQuad(Vector3(0.f, 0.f, 0.f), Vector2(200.f, 200.f), AABB2::UNIT_SQUARE_OFFCENTER, Rgba::WHITE, Vector3::DIRECTION_RIGHT, Vector3::DIRECTION_FORWARD);
	mb.FinishBuilding();
	Mesh* meshToAdd = mb.CreateMesh();
	mb.Clear();

	GameObject* ground = new GameObject();
	Material* material = AssetDB::CreateOrGetSharedMaterial("Ground");
	material->SetProperty("SPECULAR_POWER", 1.f);
	material->SetProperty("SPECULAR_AMOUNT", 0.f);

	Renderable* groundRenderable = new Renderable(Matrix44::IDENTITY, meshToAdd, material);
	ground->SetRenderable(groundRenderable);
	s_scene->AddRenderable(groundRenderable);
	m_gameObjects.push_back(ground);

	// Some random cubes for an environment
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES);
	mb.FinishBuilding();
	Mesh* buildingMesh = mb.CreateMesh();
	mb.Clear();

	int numBuildings = GetRandomIntInRange(40, 50);
	for (int buildingIndex = 0; buildingIndex < numBuildings; ++buildingIndex)
	{
		GameObject* building = new GameObject();
		Material* buildingMat = AssetDB::CreateOrGetSharedMaterial("Building");
		material->SetProperty("SPECULAR_POWER", 1.f);
		material->SetProperty("SPECULAR_AMOUNT", 0.f);

		Vector2 randomXZ = Vector2::GetRandomVector(1.f) * 50.f;
		Vector3 randomPosition = Vector3(randomXZ.x, 0.f, randomXZ.y);
		Vector3 randomScale = Vector3::GetRandomVector(GetRandomFloatInRange(15.f, 25.f));
		Matrix44 model = Matrix44::MakeModelMatrix(randomPosition, Vector3::ZERO, randomScale);

		Renderable* buildingRend = new Renderable(model, buildingMesh, buildingMat);

		building->SetRenderable(buildingRend);
		s_scene->AddRenderable(buildingRend);
		m_gameObjects.push_back(building);
	}

	// Build static Mikus for the sake of showing off lighting
	MeshGroup* mesh = AssetDB::CreateOrGetMeshGroup("Miku.obj");
	Renderable* renderable = new Renderable(Matrix44::IDENTITY, mesh, nullptr);
	
	Material* quad		= AssetDB::CreateOrGetSharedMaterial("Miku_Quad_Instanced"); 
	Material* base		= AssetDB::CreateOrGetSharedMaterial("Miku_Base_Instanced"); 
	Material* detail	= AssetDB::CreateOrGetSharedMaterial("Miku_Detail_Instanced");

	// Make a linear sampler for miku
	Sampler* sampler = new Sampler();
	sampler->Initialize(SAMPLER_FILTER_LINEAR, EDGE_SAMPLING_REPEAT);

	quad->SetSampler(0, sampler);
	base->SetSampler(0, sampler);
	detail->SetSampler(0, sampler);
	quad->SetProperty("SPECULAR_POWER", 25.0f);
	quad->SetProperty("SPECULAR_AMOUNT", 1.f);
	base->SetProperty("SPECULAR_POWER", 25.0f);
	base->SetProperty("SPECULAR_AMOUNT", 1.f);	
	detail->SetProperty("SPECULAR_POWER", 25.0f);
	detail->SetProperty("SPECULAR_AMOUNT", 1.f);

	renderable->SetSharedMaterial(quad, 0);
	renderable->SetSharedMaterial(base, 1);
	renderable->SetSharedMaterial(detail, 2);
	renderable->SetSharedMaterial(base, 3);

	renderable->ClearInstances();
	// Push the models for the instance rendering
	int numMikusThisCircle = 100;
	int numRings = 10;
	float radius = 100.f;
	for (int i = 0; i < numRings; ++i)
	{
		for (int j = 0; j < numMikusThisCircle; ++j)
		{
			float theta = ((float) j / (float) numMikusThisCircle) * 360.f;
			float x = radius * CosDegrees(theta);
			float z = radius * SinDegrees(theta);

			Vector3 position = Vector3(x, 0.f, z);
			Vector3 rotation = Vector3(0.f, -theta - 90.f, 0.f);

			Matrix44 model = Matrix44::MakeModelMatrix(position, rotation, Vector3::ONES);

			renderable->AddModelMatrix(model);
		}

		numMikusThisCircle += 50;
		radius += 10.f;
	}

	s_scene->AddRenderable(renderable);
}


//-----------------------------------------------------------------------------------------------
// Constructs and stores all the lights for the scene
//
void GameState_Playing::ConstructLights()
{
	// A directional light
	Light* d1 = Light::CreateDirectionalLight(Vector3(0.f, 5.f, 100.f), Vector3::DIRECTION_BACK, Rgba(255, 255, 0, 160));
	s_scene->AddLight(d1);

	// Spot light on camera
	m_playerSpotLight = Light::CreateConeLight(s_Player->transform.position, m_playerCamera->GetForwardVector(), 45.f, 25.f, Rgba::LIGHT_BLUE, Vector3(0.f, 0.f, 0.001f));
	s_scene->AddLight(m_playerSpotLight);

	// Visualize the directional light
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES, Rgba::YELLOW);
	mb.FinishBuilding();
	Mesh* cube = mb.CreateMesh();
	Material* mat = AssetDB::GetSharedMaterial("Default_Opaque");
	Renderable* directionalRend = new Renderable(Vector3(0.f, 5.f, 100.f), cube, mat);

	s_scene->AddRenderable(directionalRend);
}


//-----------------------------------------------------------------------------------------------
// Updates all the lights in the scene
//
void GameState_Playing::UpdateLights()
{
	// Spot light
	LightData spotLightData = m_playerSpotLight->GetLightData();
	spotLightData.m_lightDirection = m_playerCamera->GetForwardVector();
	spotLightData.m_position = s_Player->transform.position;
	m_playerSpotLight->SetLightData(spotLightData);
}


//-----------------------------------------------------------------------------------------------
// Calls update on all game objects in the game
//
void GameState_Playing::UpdateGameObjects()
{
	float deltaTime = Game::GetDeltaTime();
	int numGameObjects = (int) m_gameObjects.size();
	for (int index = 0; index < numGameObjects; ++index)
	{
		Zombie* zombie = dynamic_cast<Zombie*>(m_gameObjects[index]);
		bool isZombie = (zombie != nullptr);
		if (!isZombie || m_zombiesEnabled)
		{
			m_gameObjects[index]->Update(deltaTime);
		}
	}

	// Update Emitters
	int numEmitters = (int) m_emitters.size();
	for (int index = 0; index < numEmitters; ++index)
	{
		m_emitters[index]->Update();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks if a zombie should be spawned, and if so spawns one
//
void GameState_Playing::CheckToSpawnZombie()
{
	if (m_zombiesEnabled && m_zombieTimer->HasIntervalElapsed())
	{
		SpawnZombie();
		m_zombieTimer->DecrementByIntervalOnce();
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for collisions between bullets and zombies, and marks them for delete when appropriate
//
void GameState_Playing::CheckForCollisions()
{
	int numGameObjects = (int) m_gameObjects.size();
	for (int bulletIndex = numGameObjects - 1; bulletIndex >= 0; --bulletIndex)
	{
		Bullet* bullet = dynamic_cast<Bullet*>(m_gameObjects[bulletIndex]);
		if (bullet != nullptr)
		{
			for (int zombieIndex = numGameObjects - 1; zombieIndex >= 0; --zombieIndex)
			{
				Zombie* zombie = dynamic_cast<Zombie*>(m_gameObjects[zombieIndex]);
				if (zombie != nullptr)
				{
					if (!zombie->IsMarkedForDelete() && !bullet->IsMarkedForDelete())
					{
						// Get the distance between the two
						float distance = (bullet->transform.position - zombie->transform.position).GetLength();
						if (distance < 2.5f)
						{
							zombie->SetMarkedForDelete(true);
							bullet->SetMarkedForDelete(true);
							m_kills++;
							AddExplosionEmitter(zombie->transform);
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for GameObjects marked for deletion, and deletes those that are
//
void GameState_Playing::CheckToDeleteObjects()
{
	int numGameObjects = (int) m_gameObjects.size();
	for (int index = numGameObjects - 1; index >= 0; --index)
	{
		if (m_gameObjects[index]->IsMarkedForDelete())
		{
			// Remove the renderable from the scene
			s_scene->RemoveRenderable(m_gameObjects[index]->GetRenderable());

			// Then delete
			delete m_gameObjects[index];
			m_gameObjects.erase(m_gameObjects.begin() + index);
		}
	}

	// Clean up emitters
	int numEmitters = (int) m_emitters.size();
	for (int emitterIndex = numEmitters - 1; emitterIndex >= 0; --emitterIndex)
	{
		if (m_emitters[emitterIndex]->IsFinished())
		{
			s_scene->RemoveRenderable(m_emitters[emitterIndex]->GetRenderable());
			delete m_emitters[emitterIndex];
			m_emitters.erase(m_emitters.begin() + emitterIndex);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Renders all world space elements for the playing state
//
void GameState_Playing::RenderWorldSpace() const
{
	ForwardRenderingPath::Render(s_scene);
}


//-----------------------------------------------------------------------------------------------
// Renders all screen space elements for the playing state
//
void GameState_Playing::RenderScreenSpace() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());

	Vector2 bottomLeft = Vector2(renderer->GetUIBounds().maxs.x * 0.5f, renderer->GetUIBounds().maxs.y * 0.7f);

	AABB2 bounds = AABB2(bottomLeft, renderer->GetUIBounds().maxs);
	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Default.png");
	renderer->DrawTextInBox2D(Stringf("Kills: %i", m_kills), bounds, Vector2(1.f, 0.f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font);

	if (m_kills >= 10)
	{
		bounds.Translate(Vector2(0.f, -30.f));
		renderer->DrawTextInBox2D("The more you kill...", bounds, Vector2(1.f, 0.f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, Rgba::ORANGE);

		if (m_kills >= 20)
		{
			bounds.Translate(Vector2(0.f, -30.f));
			renderer->DrawTextInBox2D("...the faster they get...", bounds, Vector2(1.f, 0.f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, Rgba::RED);

			if (m_kills >= 50)
			{
				bounds.Translate(Vector2(0.f, -30.f));
				renderer->DrawTextInBox2D("...you can't actually die though, so I guess you win for getting this far.", bounds, Vector2(1.f, 0.f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, Rgba::GREEN);
			}
		}
	}

	bounds = renderer->GetUIBounds();
	renderer->DrawTextInBox2D("Press and hold LMB to fire\nPress 'N' to spawn a point light\nPress 'M' to toggle Zombies On/Off", bounds, Vector2(0.f, 0.9f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, Rgba::WHITE);

	std::string enabledText = (m_zombiesEnabled ? "Zombies Enabled" : "Zombies Disabled");
	Rgba color = (m_zombiesEnabled ? Rgba::RED : Rgba::GREEN);
	renderer->DrawTextInBox2D(enabledText.c_str(), bounds, Vector2(0.f, 0.f), 30.f, TEXT_DRAW_SHRINK_TO_FIT, font, color);
}


//-----------------------------------------------------------------------------------------------
// Adds a point light and renderable visualization at the given position
//
void GameState_Playing::AddPointLight(const Vector3& position)
{
	Rgba color = Rgba::GetRandomColor();
	Light* light = Light::CreatePointLight(position, color, Vector3(0.f, 0.f, 0.001f));
	s_scene->AddLight(light);

	// Build a mesh for the cube
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES, color);
	mb.FinishBuilding();
	Mesh* cube = mb.CreateMesh();
	Material* mat = AssetDB::GetSharedMaterial("Default_Opaque");

	Renderable* renderable = new Renderable(position, cube, mat);
	s_scene->AddRenderable(renderable);
}


//-----------------------------------------------------------------------------------------------
// Adds an explosion of mikus, used when a zombie dies
//
void GameState_Playing::AddExplosionEmitter(Transform& spawnTransform)
{
	Renderable* renderable = new Renderable(spawnTransform.GetModelMatrix(), AssetDB::CreateOrGetMeshGroup("Miku.obj"), nullptr);
	
	Material* quad		= AssetDB::CreateOrGetSharedMaterial("Miku_Quad_Instanced"); 
	Material* base		= AssetDB::CreateOrGetSharedMaterial("Miku_Base_Instanced"); 
	Material* detail	= AssetDB::CreateOrGetSharedMaterial("Miku_Detail_Instanced");

	// Make a linear sampler for miku
	Sampler* sampler = new Sampler();
	sampler->Initialize(SAMPLER_FILTER_LINEAR, EDGE_SAMPLING_REPEAT);

	quad->SetSampler(0, sampler);
	base->SetSampler(0, sampler);
	detail->SetSampler(0, sampler);
	quad->SetProperty("SPECULAR_POWER", 25.0f);
	quad->SetProperty("SPECULAR_AMOUNT", 1.f);
	base->SetProperty("SPECULAR_POWER", 25.0f);
	base->SetProperty("SPECULAR_AMOUNT", 1.f);	
	detail->SetProperty("SPECULAR_POWER", 25.0f);
	detail->SetProperty("SPECULAR_AMOUNT", 1.f);

	renderable->SetSharedMaterial(quad, 0);
	renderable->SetSharedMaterial(base, 1);
	renderable->SetSharedMaterial(detail, 2);
	renderable->SetSharedMaterial(base, 3);

	ParticleEmitter* emitter = new ParticleEmitter(Game::GetGameClock());;
	emitter->SetTransform(spawnTransform.position + Vector3(0.f, 2.f, 0.f), Vector3::ZERO, Vector3::ONES);
	emitter->SetRenderable(renderable);
	emitter->SetSpawnLifetimeFunction([]() { return 2.f; });
	emitter->SetSpawnVelocityFunction([]() { return 10.f * GetRandomPointOnSphere(); });
	emitter->SetSpawnAngularVelocityFunction([]() { return 360.f * GetRandomPointOnSphere(); });
	emitter->SetSpawnScaleFunction([]() { return 0.1f * Vector3::ONES; });
	emitter->SetKillWhenDone(true);
	emitter->SpawnBurst(1000);

	m_emitters.push_back(emitter);
	s_scene->AddRenderable(emitter->GetRenderable());
}


//-----------------------------------------------------------------------------------------------
// Spawns a bullet in front of the shooter passed
//
void GameState_Playing::SpawnBullet(GameObject* shooter)
{
	if (m_fireRateTimer->HasIntervalElapsed())
	{
		Vector3 fireDirection = shooter->transform.GetWorldForward();
		Bullet* bullet = new Bullet(shooter->transform);
		s_scene->AddRenderable(bullet->GetRenderable());

		m_gameObjects.push_back(bullet);

		m_fireRateTimer->DecrementByIntervalAll();
	}
}


//-----------------------------------------------------------------------------------------------
// Adds a zombie to the world 
//
void GameState_Playing::SpawnZombie()
{
	Zombie* zombie = new Zombie();
	m_gameObjects.push_back(zombie);
}
