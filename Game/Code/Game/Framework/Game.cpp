/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Audio/FFTSystem.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// The singleton instance
Game* Game::s_instance = nullptr;

void Command_PlaySongForFFTAnalysis(Command& cmd)
{
	std::string filepath;
	bool specified = cmd.GetParam("f", filepath);

	if (specified)
	{
		FFTSystem* fftSystem = Game::GetFFTSystem();
		SoundID songID = fftSystem->CreateOrGetSound(filepath);

		if (songID == MISSING_SOUND_ID)
		{
			ConsoleErrorf("Couldn't find song %s", filepath.c_str());
			return;
		}

		fftSystem->CollectFFTDataFromSong(filepath.c_str());
		ConsolePrintf(Rgba::GREEN, "Starting FFT Bin Collection on %s...please wait for playback to finish", filepath.c_str());
	}
	else
	{
		ConsoleErrorf("No file specified, use -f op");
	}
}

void Command_PerformBeatAnalysis(Command& cmd)
{
	std::string filepath;
	bool specified = cmd.GetParam("f", filepath);

	float beatWindowDuration = 0.5f;
	cmd.GetParam("w", beatWindowDuration, &beatWindowDuration);

	float delayOnDetect = 0.4f;
	cmd.GetParam("d", delayOnDetect, &delayOnDetect);

	float beatThresholdScalar = 2.2f;
	cmd.GetParam("bt", beatThresholdScalar, &beatThresholdScalar);
	
	float periodMedianThrehold = 0.1f;
	cmd.GetParam("pet", periodMedianThrehold, &periodMedianThrehold);

	float phaseMedianThreshold = 0.1f;
	cmd.GetParam("pht", phaseMedianThreshold, &phaseMedianThreshold);

	if (specified)
	{
		FFTSystem* fftSystem = Game::GetFFTSystem();

		fftSystem->PeformBeatDetectionAnalysis(filepath, beatWindowDuration, beatThresholdScalar, delayOnDetect, periodMedianThrehold, phaseMedianThreshold);

		ConsolePrintf(Rgba::GREEN, "Starting FFT Beat analysis on %s...", filepath.c_str());
	}
	else
	{
		ConsoleErrorf("No file specified, use -f op");
	}
}

void Command_RunBeatPlayback(Command& cmd)
{
	std::string songName;
	bool specified = cmd.GetParam("n", songName);

	if (!specified)
	{
		ConsoleErrorf("No name specified with -n");
		return;
	}

	Game::GetFFTSystem()->PlaySongWithBeatAnalysisData(songName.c_str());
}


//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
{
	// Clock
	m_gameClock = new Clock(Clock::GetMasterClock());
	m_gameClock->SetMaxDeltaTime((1.f / 30.f)); // Limit the time step to be no more than 1/30th a frame

	// Camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 1000.f);
	m_gameCamera->LookAt(Vector3(0.f, 5.0, -5.f), Vector3::ZERO);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// FFT System
	m_fftSystem = new FFTSystem();

	//SoundID sound = m_fftSystem->CreateOrGetSound("Data/Audio/Music/DrumSet.mp3");
// 	SoundID sound = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/50hz.mp3");
// 	SoundID sound4 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/200hz.mp3");
// 	SoundID sound2 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/1000hz.mp3");
// 	SoundID sound3 = AudioSystem::GetInstance()->CreateOrGetSound("Data/Audio/Music/5000hz.mp3");

	//m_fftSystem->PlayMusicTrackForFFT(sound, 1.0f);
// 	AudioSystem::GetInstance()->PlaySound(sound2, true);
// 	AudioSystem::GetInstance()->PlaySound(sound3, true);
// 	AudioSystem::GetInstance()->PlaySound(sound4, true);
}

	
//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_fftSystem;
	m_fftSystem = nullptr;

	delete m_gameCamera;
	m_gameCamera = nullptr;

	delete m_gameClock;
	m_gameClock = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Constructs the singleton game instance
//
void Game::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: Game::Initialize called when a Game instance already exists.");
	s_instance = new Game();

	// Set the game clock on the Renderer
	Renderer::GetInstance()->SetRendererGameClock(s_instance->m_gameClock);

	// Set up the mouse for FPS controls (Free camera)
	Mouse& mouse = InputSystem::GetMouse();

	mouse.ShowMouseCursor(false);
	mouse.LockCursorToClient(true);
	mouse.SetCursorMode(CURSORMODE_RELATIVE);

	Command::Register("fft_collect", "Collects FFT Data given the song by -f", Command_PlaySongForFFTAnalysis);
	Command::Register("fft_analyze_beat", "Analyzed beats in fft data file given in -f", Command_PerformBeatAnalysis);
	Command::Register("fft_play_beat", "Plays back the beat data for song name -n", Command_RunBeatPlayback);
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void Game::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Checks for input received this frame and updates states accordingly
//
void Game::ProcessInput()
{
	m_fftSystem->ProcessInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	m_fftSystem->BeginFrame();
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	m_fftSystem->Render();
}


//-----------------------------------------------------------------------------------------------
// Returns the game clock
//
Clock* Game::GetGameClock()
{
	return s_instance->m_gameClock;
}


//-----------------------------------------------------------------------------------------------
// Returns the camera used to render game elements
//
Camera* Game::GetGameCamera()
{
	return s_instance->m_gameCamera;
}


//-----------------------------------------------------------------------------------------------
// Returns the frame time for the game clock
//
float Game::GetDeltaTime()
{
	return s_instance->m_gameClock->GetDeltaTime();
}


//-----------------------------------------------------------------------------------------------
// Returns the FFT system of the game
//
FFTSystem* Game::GetFFTSystem()
{
	return s_instance->m_fftSystem;
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
