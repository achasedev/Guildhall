/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: July 17th, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include "Game/GameStates/GameState.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include <string>
#include <vector>

enum ePlayState
{
	PLAY_STATE_IDLE,
	PLAY_STATE_WAVE,
	PLAY_STATE_REST,
	PLAY_STATE_PAUSE,
	PLAY_STATE_DEATH,
	PLAY_STATE_VICTORY
};


class PlayState;
class CampaignDefinition;

class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing(const std::string& campaignName);
	~GameState_Playing();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;

	void TransitionToPlayState(PlayState* state);
	void PushOverrideState(PlayState* overrideState);
	void PopOverrideState();

	void PerformControllerCheck();


private:
	//-----Private Data-----

	const CampaignDefinition* m_campaignBeingPlayed = nullptr;

	PlayState* m_overrideState = nullptr;
	bool m_overrideEntered = false;

	PlayState* m_currentState = nullptr;
	PlayState* m_transitionState = nullptr;

	bool m_isTransitioning = false;
	bool m_previousTransitionStateBeforeOverride = false; // In case we push an override while we are already transitioning

};
