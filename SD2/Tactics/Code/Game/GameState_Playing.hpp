/************************************************************************/
/* File: GameState_Playing.hpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Class to represent the state when gameplay is active
/************************************************************************/
#pragma once
#include <vector>
#include <stack>
#include <queue>
#include "Game/GameState.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"

class OrbitCamera;
class BoardState;
class Actor;
class Menu;
class Rgba;
class PlayMode;
class PlayAction;
class BoardStateDefinition;

class GameState_Playing : public GameState
{
public:
	//-----Public Methods-----

	GameState_Playing(BoardStateDefinition* boardStateDefinition);
	~GameState_Playing();

	virtual void ProcessInput() override;
	virtual void Update() override;
	virtual void Render() const override;

	virtual void Enter() override;
	virtual void Leave() override;

	BoardState* GetBoardState() const;

	// Actions and Modes
	void EnqueueAction(PlayAction* action);
	void PushMode(PlayMode* mode);

	OrbitCamera* GetGameCamera();
	void TranslateCamera(const Vector3& target);
	void LerpCameraToRotation(float rotationTarget);


private:
	//-----Private Methods-----

	// Initialize
	void SetBoardStateDefinition(BoardStateDefinition* definition);

	// Update
	bool HasRunningActions() const;
	void UpdateCameraOnInput();
	void UpdateActions();
	void UpdateModes();

	bool IsTopModePause() const;

	void FinishActionsAndModes();
	void DeleteActionsAndModes();

	// Render
	void RenderWorldSpace() const;
	void RenderScreenSpace() const;


private:
	//-----Private Data-----

	BoardState*				m_boardState;		// Current board state (map, actors)
	BoardStateDefinition*	m_boardStateDefinition;

	std::stack<PlayMode*>	m_modeStack;
	std::queue<PlayAction*> m_actionQueue;

	OrbitCamera*			m_gameCamera;	
	float					m_rotationTarget;
	bool					m_isCameraRotating = false;
};
