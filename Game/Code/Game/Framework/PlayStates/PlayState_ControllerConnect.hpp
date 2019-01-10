/************************************************************************/
/* File: PlayState_ControllerConnect.hpp
/* Author: Andrew Chase
/* Date: January 9th 2019
/* Description: Class representing the state of play when a controller disconnects
/************************************************************************/
#pragma once
#include "Game/Framework/PlayStates/PlayState.hpp"

class PlayState_ControllerConnect : public PlayState
{
public:
	//-----Public Methods-----

	PlayState_ControllerConnect(int controllerIndex);
	~PlayState_ControllerConnect();

	virtual void ProcessInput() override;

	virtual bool Enter() override;
	virtual void Update() override;
	virtual bool Leave() override;

	virtual void Render_Enter() const override;
	virtual void Render() const override;
	virtual void Render_Leave() const override;


private:
	//-----Private Data-----

	IntVector3 m_menuStartPosition = IntVector3(128, 40, 160);

	int m_controllerIndex = -1;

	std::vector<std::string> m_menuText;
	bool m_resumePressed = false;

	static constexpr float CONTROLLER_TRANSITION_IN_TIME = 0.2f;
	static constexpr float CONTROLLER_TRANSITION_OUT_TIME = 0.f;

};
