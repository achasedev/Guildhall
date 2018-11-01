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

// Port the net session will run on
#define GAME_PORT 10084

// Message IDs for game messages
enum eNetGameMessage : uint8_t
{
	NET_MSG_GAME_TEST = NET_MSG_CORE_COUNT,

	// Other messages

	NET_MSG_UNRELIABLE_TEST = 128,
	NET_MSG_RELIABLE_TEST = 129,
	NET_MSG_SEQUENCE_TEST = 130
};

// Message callbacks
bool OnUnreliableTest(NetMessage* msg, const NetSender_t& sender);
bool OnReliableTest(NetMessage* msg, const NetSender_t& sender);
bool OnSequenceTest(NetMessage* msg, const NetSender_t& sender);

// Commands for testing the NetSession

//-----------------------------------------------------------------------------------------------
// Sends an add request message to the given connection index
//
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


//-----------------------------------------------------------------------------------------------
// Sends a ping message to the given connection index
//
void Command_SendPing(Command& cmd)
{
	std::string address;
	bool addressSpecified = cmd.GetParam("a", address);

	int connectionIndex = -1;
	if (!addressSpecified && !cmd.GetParam("i", connectionIndex))
	{
		ConsoleErrorf("No connection index or address specified specified");
		return;
	}

	const NetMessageDefinition_t* definition = Game::GetNetSession()->GetMessageDefinition("ping");

	if (definition == nullptr)
	{
		ConsoleErrorf("Definition does not exist on NetSession for message \"ping\"");
		return;
	}

	NetMessage* msg = new NetMessage(definition);
	msg->WriteString("Hello, World!");

	if (addressSpecified)
	{
		NetSender_t sender;
		sender.netSession = Game::GetNetSession();
		sender.address = NetAddress_t(address.c_str(), false);

		Game::GetNetSession()->SendMessageDirect(msg, sender);
		ConsolePrintf(Rgba::GREEN, "Sent a ping to address %s", address.c_str());
	}
	else
	{
		NetConnection* connection = Game::GetNetSession()->GetConnection((uint8_t)connectionIndex);

		if (connection == nullptr)
		{
			ConsoleErrorf("Could not find connection at index %i", connectionIndex);
			return;
		}
		connection->Send(msg);
		ConsolePrintf("Sent ping to connection %i", connectionIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Command for setting the NetSession simulated latency
//
void Command_SetNetSimLag(Command& cmd)
{
	float min = 0.1f;
	cmd.GetParam("min", min, &min);

	float max = min;
	cmd.GetParam("max", max, &max);

	NetSession* session = Game::GetNetSession();
	session->SetSimLatency(min, max);
}


//-----------------------------------------------------------------------------------------------
// Command for setting the NetSession simulated packet loss
//
void Command_SetNetSimLoss(Command& cmd)
{
	float loss = 0.f;
	cmd.GetParam("a", loss, &loss);

	NetSession* session = Game::GetNetSession();
	session->SetSimLoss(loss);
}


//-----------------------------------------------------------------------------------------------
// Command for sending the NetSession tick rate
//
void Command_SetSessionNetTick(Command& cmd)
{
	float hertz = 60.f;
	bool provided = cmd.GetParam("f", hertz, &hertz);

	float timeInterval;

	if (provided && hertz > 0.f)
	{
		timeInterval = (1.f / hertz);
	}
	else
	{
		timeInterval = 0;
	}

	ConsolePrintf(Rgba::GREEN, "Setting the NetSession tick rate to %f hertz", timeInterval);

	Game::GetNetSession()->SetNetTickRate(hertz);
}


//-----------------------------------------------------------------------------------------------
// Command for setting a NetConnection tick rate
//
void Command_SetConnectionNetTick(Command& cmd)
{
	int index = -1;
	cmd.GetParam("i", index);

	if (index <= -1)
	{
		ConsoleErrorf("No index (-i) specified");
		return;
	}

	float hertz = 0.f;
	bool provided = cmd.GetParam("h", hertz);

	float timeInterval;

	if (provided && hertz > 0.f)
	{
		timeInterval = (1.f / hertz);
	}
	else
	{
		timeInterval = 0.f;
	}

	ConsolePrintf(Rgba::GREEN, "Setting the NetConnection at index %i tick rate to %f between each send", index, timeInterval);

	Game::GetNetSession()->GetConnection((uint8_t)index)->SetNetTickRate(hertz);
}


//-----------------------------------------------------------------------------------------------
// Command for setting the heartbeat interval
//
void Command_SetHeartbeat(Command& cmd)
{
	float hertz = 2.f;
	cmd.GetParam("a", hertz);

	Game::GetNetSession()->SetConnectionHeartbeatInterval(hertz);

	ConsolePrintf(Rgba::GREEN, "Set the NetSession's heartbeat to %f hz", hertz);
}

#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"

struct ThreadWork_Data
{
	unsigned int connectionIndex;
	unsigned int messageCount;
	unsigned int channelIndex;
	NetAddress_t address;
	bool useAddress;
};

void ThreadWork_UnreliableTest(void* args)
{
	Stopwatch timer;

	ThreadWork_Data* data = (ThreadWork_Data*)args;

	unsigned int sent = 0;
	timer.SetInterval(0.03f);

	while (sent < data->messageCount)
	{
		if (timer.HasIntervalElapsed())
		{
			NetMessage* message = new NetMessage(Game::GetNetSession()->GetMessageDefinition("unreliable_test"));

			message->Write(sent);
			message->Write(data->messageCount);

			sent++;
			timer.SetInterval(0.03f);

			if (data->useAddress)
			{
				NetSender_t sender;
				sender.address = data->address;
				sender.netSession = Game::GetNetSession();

				Game::GetNetSession()->SendMessageDirect(message, sender);
			}
			else
			{
				NetConnection* connection = Game::GetNetSession()->GetConnection((uint8_t)data->connectionIndex);
				if (connection == nullptr)
				{
					ConsoleErrorf("No connection at %i", data->connectionIndex);
				}
				else
				{
					connection->Send(message);
				}
			}
		}
	}

	free(args);
}

void ThreadWork_ReliableTest(void* args)
{
	Stopwatch timer;

	ThreadWork_Data* data = (ThreadWork_Data*)args;

	unsigned int sent = 0;
	timer.SetInterval(0.03f);

	while (sent < data->messageCount)
	{
		if (timer.HasIntervalElapsed())
		{
			NetMessage* message = new NetMessage(Game::GetNetSession()->GetMessageDefinition("reliable_test"));

			message->Write(sent);
			message->Write(data->messageCount);

			sent++;
			timer.SetInterval(0.02f);

			if (data->useAddress)
			{
				NetSender_t sender;
				sender.address = data->address;
				sender.netSession = Game::GetNetSession();

				Game::GetNetSession()->SendMessageDirect(message, sender);
			}
			else
			{
				NetConnection* connection = Game::GetNetSession()->GetConnection((uint8_t)data->connectionIndex);
				if (connection == nullptr)
				{
					ConsoleErrorf("No connection at %i", data->connectionIndex);
				}
				else
				{
					connection->Send(message);
				}
			}
		}
	}

	free(args);
}

void ThreadWork_SequenceTest(void* args)
{
	Stopwatch timer;

	ThreadWork_Data* data = (ThreadWork_Data*)args;

	unsigned int sent = 0;
	timer.SetInterval(0.03f);

	while (sent < data->messageCount)
	{
		if (timer.HasIntervalElapsed())
		{
			NetMessage* message = new NetMessage(Game::GetNetSession()->GetMessageDefinition("sequence_test"));
			message->AssignSequenceChannelID((uint8_t)data->channelIndex);

			message->Write(sent);
			message->Write(data->messageCount);

			sent++;
			timer.SetInterval(0.02f);

			if (data->useAddress)
			{
				NetSender_t sender;
				sender.address = data->address;
				sender.netSession = Game::GetNetSession();

				Game::GetNetSession()->SendMessageDirect(message, sender);
			}
			else
			{
				NetConnection* connection = Game::GetNetSession()->GetConnection((uint8_t)data->connectionIndex);
				if (connection == nullptr)
				{
					ConsoleErrorf("No connection at %i", data->connectionIndex);
				}
				else
				{
					connection->Send(message);
				}
			}
		}
	}

	free(args);
}

void Command_UnreliableTest(Command& cmd)
{
	unsigned int connectionIndex = INVALID_CONNECTION_INDEX;
	unsigned int messageCount = 10;

	std::string address;
	bool addressSpecifed = cmd.GetParam("a", address);
	bool connectionSpecified = cmd.GetParam("i", connectionIndex);

	if (!connectionSpecified && !addressSpecifed)
	{
		ConsoleErrorf("No connection index or address specified");
		return;
	}

	cmd.GetParam("c", messageCount, &messageCount);


	ThreadWork_Data* data = (ThreadWork_Data*)malloc(sizeof(ThreadWork_Data));
	data->messageCount = messageCount;
	data->connectionIndex = connectionIndex;
	data->address = NetAddress_t(address.c_str(), false);
	data->useAddress = addressSpecifed;

	Thread::CreateAndDetach(ThreadWork_UnreliableTest, data);
}


void Command_ReliableTest(Command& cmd)
{
	unsigned int connectionIndex = INVALID_CONNECTION_INDEX;
	unsigned int messageCount = 10;

	std::string address;
	bool addressSpecifed = cmd.GetParam("a", address);
	bool connectionSpecified = cmd.GetParam("i", connectionIndex);

	if (!connectionSpecified && !addressSpecifed)
	{
		ConsoleErrorf("No connection index or address specified");
		return;
	}

	cmd.GetParam("c", messageCount, &messageCount);


	ThreadWork_Data* data = (ThreadWork_Data*)malloc(sizeof(ThreadWork_Data));
	data->messageCount = messageCount;
	data->connectionIndex = connectionIndex;
	data->address = NetAddress_t(address.c_str(), false);
	data->useAddress = addressSpecifed;

	Thread::CreateAndDetach(ThreadWork_ReliableTest, data);
}


void Command_SequenceTest(Command& cmd)
{
	unsigned int connectionIndex = INVALID_CONNECTION_INDEX;
	unsigned int messageCount = 10;

	std::string address;
	bool addressSpecifed = cmd.GetParam("a", address);
	bool connectionSpecified = cmd.GetParam("i", connectionIndex);

	if (!connectionSpecified && !addressSpecifed)
	{
		ConsoleErrorf("No connection index or address specified");
		return;
	}

	cmd.GetParam("c", messageCount, &messageCount);

	unsigned int channelIndex = 0;
	cmd.GetParam("ch", channelIndex, &channelIndex);

	ThreadWork_Data* data = (ThreadWork_Data*)malloc(sizeof(ThreadWork_Data));
	data->messageCount = messageCount;
	data->connectionIndex = connectionIndex;
	data->address = NetAddress_t(address.c_str(), false);
	data->useAddress = addressSpecifed;
	data->channelIndex = channelIndex;

	Thread::CreateAndDetach(ThreadWork_SequenceTest, data);
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
	RegisterGameMessages();
	m_netSession->Bind(GAME_PORT, 10);
}


//-----------------------------------------------------------------------------------------------
// Destructor - clean up any allocated members (private)
//
Game::~Game()
{
	delete m_netSession;
	m_netSession = nullptr;

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

	Command::Register("net_sim_lag", "Sets the simulated latency of the game net session", Command_SetNetSimLag);
	Command::Register("net_sim_loss", "Sets the simulated packet loss of the game net session", Command_SetNetSimLoss);

	Command::Register("net_set_session_send_rate", "Sets the NetSession's network tick rate", Command_SetSessionNetTick);
	Command::Register("net_set_connection_send_rate", "Sets the connection's tick rate at the specified index", Command_SetConnectionNetTick);

	Command::Register("net_set_heartbeat", "Sets the NetSession's heartbeat", Command_SetHeartbeat);

	Command::Register("unreliable_test", "Sends unreliable messages on a fixed interval for testing", Command_UnreliableTest);
	Command::Register("reliable_test", "Sends reliable messages on a fixed interval for testing", Command_ReliableTest);
	Command::Register("sequence_test", "Sends in-order messages on a fixed interval for testing", Command_SequenceTest);
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
	m_netSession->RenderDebugInfo();
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


//-----------------------------------------------------------------------------------------------
// Registers all game net messages to the game NetSession
//
void Game::RegisterGameMessages()
{
	m_netSession->RegisterMessageDefinition(NET_MSG_UNRELIABLE_TEST, "unreliable_test", OnUnreliableTest);
	m_netSession->RegisterMessageDefinition(NET_MSG_RELIABLE_TEST, "reliable_test", OnReliableTest, NET_MSG_OPTION_RELIABLE);
	m_netSession->RegisterMessageDefinition(NET_MSG_SEQUENCE_TEST, "sequence_test", OnSequenceTest, (eNetMessageOption)(NET_MSG_OPTION_RELIABLE | NET_MSG_OPTION_IN_ORDER));
}


//-----------------------------------------------------------------------------------------------
// Message callbacks
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// For testing unreliable messages that require a connection
//
bool OnUnreliableTest(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);
	UNUSED(sender);

	uint32_t messageNumber, messageCount;

	msg->Read(messageNumber);
	msg->Read(messageCount);

	ConsolePrintf("UnreliableTest message received: (%i, %i)", messageNumber, messageCount);

	return true;
}

#include "Engine/Core/LogSystem.hpp"
//-----------------------------------------------------------------------------------------------
// For testing reliable traffic
//
bool OnReliableTest(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);
	UNUSED(sender);

	ConsolePrintf("ReliableTest message received, ID: %i", msg->GetReliableID());
	LogTaggedPrintf("NET", "ReliableTest message received, ID: %i", msg->GetReliableID());

	return true;
}

//-----------------------------------------------------------------------------------------------
// For testing in order traffic
//
bool OnSequenceTest(NetMessage* msg, const NetSender_t& sender)
{
	UNUSED(msg);
	UNUSED(sender);

	ConsolePrintf("SequenceTest message received - Reliable ID: %i | Sequence Channel: %i | Sequence ID: %i", msg->GetReliableID(), msg->GetSequenceChannelID(), msg->GetSequenceID());
	LogTaggedPrintf("NET", "SequenceTest message received - Reliable ID: %i | Sequence Channel: %i | Sequence ID: %i", msg->GetReliableID(), msg->GetSequenceChannelID(), msg->GetSequenceID());

	return true;
}
