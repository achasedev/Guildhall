/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/GameStates/GameState_Loading.hpp"

#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Networking/NetConnection.hpp"

#include "Engine/Networking/Socket.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"

// Test commands
void Command_UDPTestStart(Command& cmd)
{
	Game::GetInstance()->m_test = new UDPTest();
	Game::GetInstance()->m_test->start();
}

void Command_UDPTestStop(Command& cmd)
{
	if (Game::GetInstance()->m_test != nullptr)
	{
		Game::GetInstance()->m_test->stop();
		delete Game::GetInstance()->m_test;
		Game::GetInstance()->m_test = nullptr;
		ConsolePrintf(Rgba::GREEN, "Test stopped");
	}
	else
	{
		ConsoleErrorf("It's already stopped");
	}
}

void Command_UDPTestSend(Command& cmd)
{
	if (Game::GetInstance()->m_test == nullptr)
	{
		ConsoleErrorf("Start it first...");
		return;
	}

	std::string address;
	cmd.GetParam("a", address);

	if (address.size() == 0)
	{
		ConsoleErrorf("No address specified");
		return;
	}

	std::string message;
	cmd.GetParam("m", message);

	if (message.size() == 0)
	{
		ConsoleErrorf("No message specified");
		return;
	}

	NetAddress_t addr(address.c_str());

	ConsolePrintf("Sending Message");
	Game::GetInstance()->m_test->send_to(addr, message.c_str(), (unsigned int)message.size());
	ConsolePrintf(Rgba::GREEN, "Message Sent");
}


bool OnPing(NetMessage* msg, NetConnection* sender)
{
	std::string str;
	msg->ReadString(str);

	ConsolePrintf("Received ping from %s: %s", sender->GetTargetAddress().ToString().c_str(), str.c_str());

	// Respond with a pong
	NetMessage message("pong");
	int amountSent = sender->Send(&message);

	// all messages serve double duty
	// do some work, and also validate
	// if a message ends up being malformed, we return false
	// to notify the session we may want to kick this connection; 
	return (amountSent >= 0);
}

bool OnPong(NetMessage* msg, NetConnection* sender)
{
	std::string str;
	msg->ReadString(str);

	ConsolePrintf("Received pong from %s: %s", sender->GetTargetAddress().ToString().c_str(), str.c_str());

	return true;
}

bool OnAdd(NetMessage* msg, NetConnection* sender)
{
	float a;
	float b;

	// Adding requires two values, ensure we can read them
	if (msg->Read(a) <= 0 || msg->Read(b) <= 0)
	{
		return false;
	}

	float sum = a + b;

	std::string result = Stringf("Add: %f + %f = %f", a, b, sum);
	ConsolePrintf(result.c_str());

	// Send a response, don't worry about checking if it sent
	NetMessage message(result.c_str());
	sender->Send(&message);

	return true;
}

//-----------------------------------------------------------------------------------------------
//--------------------------------- Game Class --------------------------------------------------
//-----------------------------------------------------------------------------------------------


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

	// Camera
	Renderer* renderer = Renderer::GetInstance();
	m_gameCamera = new Camera();
	m_gameCamera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_gameCamera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_gameCamera->SetProjectionPerspective(45.f, 0.1f, 10000.f);
	m_gameCamera->LookAt(Vector3(0.f, 200.f, -500.0f), Vector3(0.f, 200.f, 0.f));

	// Render Scene
	m_renderScene = new RenderScene("Game Scene");
	m_renderScene->AddCamera(m_gameCamera);

	// Net Session
	m_netSession = new NetSession(128);

	// Link messages to callbacks
	m_netSession->RegisterMessageCallback("ping", OnPing);
	m_netSession->RegisterMessageCallback("pong", OnPong);
	m_netSession->RegisterMessageCallback("add", OnAdd);

	m_netSession->AddBinding(GAME_PORT);

	Command::Register("udptest_start", "Starts the UDP test", Command_UDPTestStart);
	Command::Register("udptest_stop", "Stops the UDP test", Command_UDPTestStop);
	Command::Register("udptest_send", "Sends a message for the UDP test", Command_UDPTestSend);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_renderScene;
	m_renderScene = nullptr;
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
	m_currentState->ProcessInput();
}

#include "Engine/Networking/Socket.hpp"

//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	// Check for state change
	CheckToUpdateGameState();

	// Update the current state
	m_currentState->Update();

	if (m_test != nullptr)
	{
		m_test->update();
	}
}


//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	m_currentState->Render();
}


//-----------------------------------------------------------------------------------------------
// Sets the pending state flag to the one given, so the next frame the game will switch to the
// given state
//
void Game::TransitionToGameState(GameState* newState)
{
	s_instance->m_pendingState = newState;
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
// Returns the game's render scene
//
RenderScene* Game::GetRenderScene()
{
	return s_instance->m_renderScene;
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton Game instance
//
Game* Game::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Checks if there is a pending 
//
void Game::CheckToUpdateGameState()
{
	// Have a state pending
	if (m_pendingState != nullptr)
	{
		if (m_currentState != nullptr)
		{
			// Leave and destroy current
			m_currentState->Leave();
			delete m_currentState;
		}

		// Set new as current
		m_currentState = m_pendingState;
		m_pendingState = nullptr;

		// Enter the new state
		m_currentState->Enter();
	}
}
