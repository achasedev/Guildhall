/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: May 21st, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Game/GameStates/GameState.hpp"

class Map;
class Player;
class Camera;
class GameObject;
class RenderScene;
class ParticleEmitter;

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

	Map* GetMap() const;


private:
	//-----Private Methods-----


	
private:
	//-----Private Data-----

	std::vector<GameObject*> m_gameObjects;

	Map* m_map;
	Player* m_player;
};
