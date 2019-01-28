/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Loading.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// The singleton instance
Game* Game::s_instance = nullptr;

//-----------------------------------------------------------------------------------------------
// Default constructor, initialize any game members here (private)
//
Game::Game()
	: m_currentState(new GameState_Loading())
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

	SetupDSPForTrack("Data/Audio/Music/Test.mp3");
}


//-----------------------------------------------------------------------------------------------
// Rebuilds the bar mesh for the FFT analysis
//
void Game::UpdateBarMesh()
{
	AudioSystem* audioSystem = AudioSystem::GetInstance();
	FMOD::System* fmodSystem = audioSystem->GetFMODSystem();
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	FMOD::DSP* dsp = nullptr;
	masterChannelGroup->getDSP(FMOD_CHANNELCONTROL_DSP_HEAD, &dsp);

	void* spectrumData = nullptr;
	dsp->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&spectrumData, 0, 0, 0);
	FMOD_DSP_PARAMETER_FFT* fft = (FMOD_DSP_PARAMETER_FFT*)spectrumData;

	Renderer* renderer = Renderer::GetInstance();
	AABB2 bounds = renderer->GetUIBounds();

	m_maxValue = 0.f;
	m_zeroCount = 0;
	m_sumOfValues = 0.f;

	m_numChannels = fft->numchannels;
	m_numSegmentsTotal = fft->length / 2;
	m_numSegmentsBeingAnalyzed = m_numSegmentsTotal / FFT_WINDOW_FRACTION_DIVISOR;

	if (fft != nullptr)
	{
		float boxWidth = bounds.GetDimensions().x / (float)m_numSegmentsBeingAnalyzed;
		AABB2 baseBoxBounds = AABB2(Vector2(10.f), Vector2(10.f, 0.f) + Vector2(boxWidth, bounds.maxs.y - 5.f * FONT_HEIGHT));

		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		for (int i = 0; i < m_numSegmentsBeingAnalyzed; ++i)
		{
			float value = fft->spectrum[0][i] + fft->spectrum[1][i];
			if (AreMostlyEqual(value, 0.f))
			{
				m_zeroCount++;
			}

			m_maxValue = MaxFloat(m_maxValue, value);
			m_sumOfValues += value;

			AABB2 currBoxBounds = baseBoxBounds;
			currBoxBounds.maxs.y = 2.f * value * baseBoxBounds.GetDimensions().y + 10.f;

			mb.Push2DQuad(currBoxBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::RED);

			baseBoxBounds.Translate(Vector2(baseBoxBounds.GetDimensions().x, 0.f));
		}


		mb.FinishBuilding();
		mb.UpdateMesh(m_barMesh);
	}
}

	
//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_gameCamera;
	m_gameCamera = nullptr;
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
	if (m_currentState != nullptr)
	{
		m_currentState->ProcessInput();
	}
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
// 	if (m_gameStateState == GAME_STATE_TRANSITIONING_OUT)
// 	{
// 		// Update on leave of the current state
// 		if (m_currentState != nullptr)
// 		{
// 			bool leaveFinished = m_currentState->Leave();
// 
// 			if (leaveFinished)
// 			{
// 				delete m_currentState;
// 				m_currentState = m_transitionState;
// 				m_transitionState = nullptr;
// 
// 				m_currentState->StartEnterTimer();
// 				m_gameStateState = GAME_STATE_TRANSITIONING_IN;
// 			}
// 		}
// 	}
// 	
// 	if (m_gameStateState == GAME_STATE_TRANSITIONING_IN) // Update on enter of the transition state
// 	{
// 		bool enterFinished = m_currentState->Enter();
// 
// 		if (enterFinished)
// 		{
// 			m_gameStateState = GAME_STATE_UPDATING;
// 		}
// 	}
// 	
// 	if (m_gameStateState == GAME_STATE_UPDATING)
// 	{
// 		m_currentState->Update();
// 	}

	UpdateBarMesh();
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
// 	switch (m_gameStateState)
// 	{
// 	case GAME_STATE_TRANSITIONING_IN:
// 		m_currentState->Render_Enter();
// 		break;
// 	case GAME_STATE_UPDATING:
// 		m_currentState->Render();
// 		break;
// 	case GAME_STATE_TRANSITIONING_OUT:
// 		m_currentState->Render_Leave();
// 		break;
// 	default:
// 		break;
// 	}
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 bounds = renderer->GetUIBounds();
	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->DrawTextInBox2D(Stringf("Number of Channels: %i", m_numChannels), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -FONT_HEIGHT));

	renderer->DrawTextInBox2D(Stringf("Number of Entries shown: %i (out of %i)", m_numSegmentsBeingAnalyzed, m_numSegmentsTotal), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -FONT_HEIGHT));

	renderer->DrawTextInBox2D(Stringf("Frequency span per entry: %f hz", 44100.f / (float)m_numSegmentsTotal), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -FONT_HEIGHT));

	renderer->DrawTextInBox2D(Stringf("Max Value: %f", m_maxValue), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -FONT_HEIGHT));

	renderer->DrawTextInBox2D(Stringf("Zero Count: %i", m_zeroCount), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -FONT_HEIGHT));

	renderer->DrawTextInBox2D(Stringf("Sum of Values: %f", m_sumOfValues), bounds, Vector2::ZERO, FONT_HEIGHT, TEXT_DRAW_OVERRUN, font);

	renderer->DrawMesh(&m_barMesh);
}


//-----------------------------------------------------------------------------------------------
// Loads the given track from file and creates a FFT DSP for it
//
bool Game::SetupDSPForTrack(const std::string& audioPath)
{
	// Variables
	AudioSystem* audioSystem = AudioSystem::GetInstance();
	SoundID sound = audioSystem->CreateOrGetSound(audioPath);

	if (sound == MISSING_SOUND_ID)
	{
		ConsoleErrorf("Couldn't load sound at path %s", audioPath.c_str());
		return false;
	}

	// FMOD variables
	FMOD::System* fmodSystem = audioSystem->GetFMODSystem();
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	FMOD::DSP* dsp = nullptr;

	// Get the master channel group
	FMOD_RESULT result = fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	if (result != FMOD_OK)
	{
		ConsoleErrorf("Couldn't get the master channel group");
		return false;
	}

	// Create and setup the FFT DSP, assigning it to the master channel group
	result = fmodSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &dsp);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't create the DSP");

	result = dsp->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window type parameter");

	result = dsp->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 2 * FFT_WINDOW_SIZE);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window size parameter");

	result = masterChannelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, dsp);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't ADD the DSP to the master channel group");

	// Start playing the song
	audioSystem->PlaySound(sound);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the game state of the Game instance
//
GameState* Game::GetGameState() const
{
	return m_currentState;
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_transitionState = newState;
	s_instance->m_gameStateState = GAME_STATE_TRANSITIONING_OUT;

	if (s_instance->m_currentState != nullptr)
	{
		s_instance->m_currentState->StartLeaveTimer();
	}
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
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}
