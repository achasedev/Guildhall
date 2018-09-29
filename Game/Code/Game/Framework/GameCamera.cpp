/************************************************************************/
/* File: GameCamera.cpp
/* Author: Andrew Chase
/* Date: September 28th 2018
/* Description: Implementation of the GameCamera class
/************************************************************************/
#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCamera.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
GameCamera::GameCamera()
	: m_offsetDirection(Vector3(0.f, 1.f, -1.f))
	, m_offsetDistance(200.f)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
GameCamera::~GameCamera()
{

}


//-----------------------------------------------------------------------------------------------
// Updates the camera to be positioned relative to the players' locations
//
void GameCamera::UpdatePositionBasedOnPlayers()
{
	Player** players = Game::GetPlayers();

	// Average the player's locations
	Vector3 targetPos = Vector3::ZERO;
	float playerCount = 0.f;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (players[i] != nullptr)
		{
			targetPos += players[i]->GetPosition();
			playerCount += 1.0f;
		}
	}

	targetPos /= playerCount;

	Vector3 newPos = targetPos + m_offsetDirection * m_offsetDistance;
	LookAt(newPos, targetPos);
}
