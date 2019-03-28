/************************************************************************/
/* File: Game.hpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Class used for managing and updating game objects and 
/*              mechanics
/************************************************************************/
#pragma once
#include <vector>
#include "Game/Framework/Leaderboard.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

class Clock;
class World;
class CampaignManager;
class GameCamera;
class Player;
class GameState;
class VoxelGrid;
class VoxelFont;

#define MAX_PLAYERS (4)
#define PLAYER_DEATH_SCORE_PENALTY (-2000)

enum eGameStateState
{
	GAME_STATE_TRANSITIONING_IN,
	GAME_STATE_UPDATING,
	GAME_STATE_TRANSITIONING_OUT
};


class Game
{
	
public:
	friend class GameState_Playing;
	friend class GameState_Loading;
	friend class GameState_MainMenu;

	//-----Public Methods-----

	static void Initialize();
	static void ShutDown();
	
	void ProcessInput();				// Process all input this frame
	void Update();						// Updates all game object states, called each frame
	void Render() const;				// Renders all game objects to screen, called each frame

	GameState* GetGameState() const;

	static Game*				GetInstance();
	static void					TransitionToGameState(GameState* newState);

	static VoxelFont*			GetMenuFont();
	static VoxelGrid*			GetVoxelGrid();
	static Clock*				GetGameClock();
	static GameCamera*			GetGameCamera();
	static float				GetDeltaTime();
	static World*				GetWorld();
	static void 				SetWorld(World* world);
	static Player**				GetPlayers();
	static int					GetCurrentPlayerCount();
	static const Leaderboard&	GetLeaderboardByName(const std::string& leaderboardName);
	static const Leaderboard&	GetLeaderboardForCurrentCampaign();
	static CampaignManager*		GetCampaignManager();

	static void					ResetScore();
	static void					AddPointsToScore(int pointsToAdd);
	static void					UpdateLeaderboardWithCurrentScore();
	static int					GetScore();

	static void					DrawPlayerHUD();
	static void					DrawHeading(const std::string& headingText, const IntVector3& drawCoords, const Vector3& alignment);

	static void					DrawScore();
	static void					DrawEnemyCountRemaining();
	static void					DrawStageNumber();

	static bool					AreAllPlayersInitialized();
	static void					RescaleDifficultyBasedOnCurrentPlayerCount();

	static void					PlayBGM(const std::string& filename, bool fadeIn = true, bool loop = true);
	static void					SetBGMVolume(float newVolume, bool transitionTo = true);
	static void					PlaySystemSound(const std::string& systemSoundName);


private:
	//-----Private Methods-----
	
	Game();
	~Game();
	Game(const Game& copy) = delete;

	void UpdateMusicCrossfade();
	void UpdateDisplayedScore();

	void LoadLeaderboardsFromFile();
	void WriteLeaderboardsToFile();


private:
	//-----Private Data-----

	bool			m_doneLoading = false;
	GameState*		m_currentState = nullptr;
	GameState*		m_transitionState = nullptr;
	eGameStateState m_gameStateState = GAME_STATE_TRANSITIONING_IN;

	Clock*				m_gameClock = nullptr;
	World*				m_world = nullptr;
	CampaignManager*	m_campaignManager = nullptr;
	VoxelGrid*			m_voxelGrid = nullptr;
	Player*				m_players[MAX_PLAYERS];

	// Camera
	GameCamera*			m_gameCamera = nullptr;

	// In-game hud
	VoxelFont* m_hudFont = nullptr;
	VoxelFont* m_menuFont = nullptr;

	// Gameplay
	float	m_actualScore = 0.f;
	float m_displayedScore = 0.f; // Stored as float to allow it to change independent of framerate, i.e. increase by 0.5 points this frame

	std::map<std::string, Leaderboard> m_campaignLeaderboards;

	// Audio
	SoundPlaybackID		m_trackTendingToTarget = MISSING_SOUND_ID;
	SoundPlaybackID		m_trackTendingToZero = MISSING_SOUND_ID;
	Stopwatch			m_musicCrossfadeTimer;
	bool				m_musicCrossfading = false;

	float				m_targetMusicVolume = 1.0f;
	float				m_tendingTargetCurrentVolume = 0.f;
	float				m_tendingZeroCurrentVolume = 0.f;

	static constexpr float MUSIC_CROSSFADE_DURATION = 1.0f;

	std::map<std::string, SoundID> m_systemSounds;

	static Game* s_instance;			// The singleton Game instance

};
