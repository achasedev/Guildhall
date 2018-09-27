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

#define GAME_PORT 10083

bool OnPing(NetMessage* msg, NetConnection* sender)
{
	std::string str;
	msg->ReadString(str);

	ConsolePrintf("Received ping from %s: %s", sender->GetAddress().ToString().c_str(), str.c_str());

	// Respond with a pong
	uint8_t definitionIndex;
	if (!Game::GetNetSession()->GetMessageDefinitionIndex("pong", definitionIndex))
	{
		ConsoleErrorf("OnPing couldn't find definition for message named \"pong\"");
		return false;
	}

	NetMessage message(definitionIndex);
	sender->Send(&message);

	// all messages serve double duty
	// do some work, and also validate
	// if a message ends up being malformed, we return false
	// to notify the session we may want to kick this connection; 
	return true;
}

bool OnPong(NetMessage* msg, NetConnection* sender)
{
	std::string str;
	msg->ReadString(str);

	ConsolePrintf("Received pong from %s: %s", sender->GetAddress().ToString().c_str(), str.c_str());

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

	return true;
}


void Command_AddConnection(Command& cmd)
{
	int index = -1;
	if (!cmd.GetParam("i", index))
	{
		ConsoleErrorf("No index specified");
		return;
	}

	if (index < 0)
	{
		ConsoleErrorf("Invalid index");
		return;
	}

	std::string addr;
	if (!cmd.GetParam("a", addr))
	{
		ConsoleErrorf("No address specified");
		return;
	}

	NetAddress_t netAddr(addr.c_str());
	NetSession* session = Game::GetNetSession();

	if (session == nullptr)
	{
		ConsoleErrorf("No Session set up in game");
		return;
	}

	bool added = session->AddConnection((uint8_t)index, netAddr);

	if (added)
	{
		ConsolePrintf(Rgba::GREEN, "Connection to %s added at index %i", addr.c_str(), (uint8_t)index);
	}
	else
	{
		ConsoleErrorf("Couldn't add connection to %s at index %i", addr.c_str(), (uint8_t)index);
	}
}


void Command_SendPing(Command& cmd)
{
	int connectionIndex = -1;
	if (!cmd.GetParam("i", connectionIndex))
	{
		ConsoleErrorf("No connection index specified");
		return;
	}

	NetConnection* connection = Game::GetNetSession()->GetConnection(connectionIndex);

	if (connection == nullptr)
	{
		ConsoleErrorf("Could not find connection at index %i", connectionIndex);
		return;
	}

	uint8_t definitionIndex;
	bool definitionExists = Game::GetNetSession()->GetMessageDefinitionIndex("ping", definitionIndex);

	if (!definitionExists)
	{
		ConsoleErrorf("Definition does not exist on NetSession for message \"ping\"");
		return;
	}

	NetMessage* msg = new NetMessage(definitionIndex);
	msg->WriteString("Hello, World!");

	connection->Send(msg);
}

void Command_SendAdd(Command& cmd)
{
	int connectionIndex = -1;
	if (!cmd.GetParam("i", connectionIndex))
	{
		ConsoleErrorf("No connection index specified");
		return;
	}

	float a = 1.0f;
	float b = 1.0f;

	cmd.GetParam("a", a, &a);
	cmd.GetParam("b", b, &b);

	NetConnection* connection = Game::GetNetSession()->GetConnection(connectionIndex);

	if (connection == nullptr)
	{
		ConsoleErrorf("Could not find connection at index %i", connectionIndex);
		return;
	}

	uint8_t definitionIndex;
	bool definitionExists = Game::GetNetSession()->GetMessageDefinitionIndex("ping", definitionIndex);

	if (!definitionExists)
	{
		ConsoleErrorf("Definition does not exist on NetSession for message \"ping\"");
		return;
	}

	NetMessage* msg = new NetMessage(definitionIndex);

	msg->Write(a);
	msg->Write(b);

	connection->Send(msg);
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
	m_netSession = new NetSession();

	// Link messages to callbacks
	m_netSession->RegisterMessageDefinition("ping", OnPing);
	m_netSession->RegisterMessageDefinition("pong", OnPong);
	m_netSession->RegisterMessageDefinition("add", OnAdd);

	m_netSession->Bind(GAME_PORT, 10);
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

	Command::Register("add_connection", "Adds a connection to the game session for the given index and address", Command_AddConnection);
	Command::Register("send_ping", "Sends a ping on the current net session to the given connection index", Command_SendPing);
	Command::Register("send_add", "Sends an add message the given index", Command_SendAdd);
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
	m_netSession->ProcessIncoming();

	// Check for state change
	CheckToUpdateGameState();

	// Update the current state
	m_currentState->Update();

	m_netSession->ProcessOutgoing();
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
// Returns the net session of the instance
//
NetSession* Game::GetNetSession()
{
	return s_instance->m_netSession;
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
