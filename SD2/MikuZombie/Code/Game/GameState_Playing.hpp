/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/GameState.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"

class Camera;
class Renderable;
class RenderScene;
class Light;
class GameObject;
class ParticleEmitter;
class Player;
class Stopwatch;
class Transform;

class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing();
	~GameState_Playing();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;

	static RenderScene* GetRenderScene();
	static Player*		GetPlayer();


private:
	//-----Private Methods-----

	// Initialization
	void InitializeRenderScene();
		void BuildGeometry();
		void ConstructLights();

	// Update
	void UpdateLights();
	void UpdateGameObjects();
	void CheckToSpawnZombie();
	void CheckForCollisions();
	void CheckToDeleteObjects();

	// Render
	void RenderWorldSpace() const;
	void RenderScreenSpace() const;

	// Utility
	void AddPointLight(const Vector3& position);
	void AddExplosionEmitter(Transform& position);
	void SpawnBullet(GameObject* shooter);
	void SpawnZombie();

	
private:
	//-----Private Data-----

	Camera* m_playerCamera;
	Light* m_playerSpotLight;

	std::vector<GameObject*> m_gameObjects;
	std::vector<ParticleEmitter*> m_emitters;

	Stopwatch* m_zombieTimer;
	Stopwatch* m_fireRateTimer;

	float m_spawnRate = ZOMBIE_INITIAL_SPAWN_RATE;
	unsigned int m_kills = 0;
	bool m_zombiesEnabled = true;

	static RenderScene* s_scene;
	static Player* s_Player;			// For convenience

	static constexpr float PLAYER_FIRERATE = 8.f;
	static constexpr float ZOMBIE_INITIAL_SPAWN_RATE = 1.f;
	static constexpr float ZOMBIE_SPAWN_RATE_ACCELERATION = 0.05f;
};
