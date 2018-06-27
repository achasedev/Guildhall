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
#include "Engine/Audio/AudioSystem.hpp"

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


private:
	//-----Private Methods-----

	void RenderUI() const;

	
private:
	//-----Private Data-----

	bool m_songPlaying = false;
	SoundPlaybackID m_song;

	AABB2 m_crosshairBounds;
	AABB2 m_reloadTimerBounds;
};
