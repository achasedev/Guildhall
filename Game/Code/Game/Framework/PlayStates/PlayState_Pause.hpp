/************************************************************************/
/* File: PlayState_Pause.hpp
/* Author: Andrew Chase
/* Date: January 7th 2019
/* Description: Class representing the state of play when one player pauses
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_Pause : public PlayState
{
public:
	//-----Public Methods-----

	PlayState_Pause(Player* pausingPlayer);
	~PlayState_Pause();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;


private:
	//-----Private Methods-----

	void ProcessSelection();


private:
	//-----Private Data-----

	Player* m_playerThatPaused = nullptr;
	int m_cursorIndex = 2;
	std::vector<std::string> m_menuText;
	bool m_resumePressed = false;

	static constexpr float PAUSE_TRANSITION_IN_TIME = 1.0f;
	static constexpr float PAUSE_TRANSITION_OUT_TIME = 1.0f;

};
