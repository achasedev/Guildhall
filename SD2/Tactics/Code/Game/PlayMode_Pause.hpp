/************************************************************************/
/* File: PlayMode_Pause.hpp
/* Author: Andrew Chase
/* Date: March 20th, 2018
/* Description: Mode for handling game pause
/************************************************************************/
#pragma once
#include "Game/PlayMode.hpp"
#include "Engine/Math/AABB2.hpp"


class Stopwatch;

class PlayMode_Pause : public PlayMode
{
public:
	//-----Public Methods-----

	PlayMode_Pause();

	// Virtual Methods
	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void RenderWorldSpace() const override;
	virtual void RenderScreenSpace() const override;
	virtual void Exit() override;


private:
	//-----Private Data-----

	bool m_fadingIn = true;
	Stopwatch* m_stopwatch;

	AABB2 m_textBounds;

	static const float PAUSE_FADE_TIME;

};
