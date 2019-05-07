/************************************************************************/
/* File: Game.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Game class for general gameplay management
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/CountJob.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Camera.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/JobSystem/JobSystem.hpp"
#include "Engine/Core/EventSystem/EventSystem.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

// The singleton instance
Game* Game::s_instance = nullptr;

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
	m_gameCamera->LookAt(Vector3(10.f, 10.f, 10.f), Vector3::ZERO, Vector3::Z_AXIS);

	DebugRenderSystem::SetWorldCamera(m_gameCamera);

	// Event System testing
	EventSystem* eventSystem = EventSystem::GetInstance();
	eventSystem->SubscribeEventCallbackFunction("Test", Game::EventSystemStaticCallback);
	eventSystem->SubscribeEventCallbackObjectMethod("Test", &Game::EventSystemObjectMethodCallback, *this);
	eventSystem->SubscribeEventCallbackFunction("Test", EventSystemCCallback);
}


//-----------------------------------------------------------------------------------------------
// Checks for input that toggles event subscriber state
//
void Game::ProcessEventSystemInput()
{
	if (!m_eventFiredTimer.HasIntervalElapsed())
	{
		return;
	}

	InputSystem* input = InputSystem::GetInstance();
	EventSystem* eventSystem = EventSystem::GetInstance();

	// For testing the event system
	if (input->WasKeyJustPressed('Y'))
	{
		m_eventFiredTimer.SetInterval(Game::EVENT_FIRED_DURATION);
		FireEvent("Test");
	}

	if (input->WasKeyJustPressed('U'))
	{
		m_staticFunctionSubscribed = !m_staticFunctionSubscribed;

		if (m_staticFunctionSubscribed)
		{
			eventSystem->SubscribeEventCallbackFunction("Test", Game::EventSystemStaticCallback);
		}
		else
		{
			eventSystem->UnsubscribeEventCallbackFunction("Test", Game::EventSystemStaticCallback);
		}
	}

	if (input->WasKeyJustPressed('I'))
	{
		m_objectMethodSubscribed = !m_objectMethodSubscribed;

		if (m_objectMethodSubscribed)
		{
			eventSystem->SubscribeEventCallbackObjectMethod("Test", &Game::EventSystemObjectMethodCallback, *this);
		}
		else
		{
			eventSystem->UnsubscribeEventCallbackObjectMethod("Test", &Game::EventSystemObjectMethodCallback, *this);
		}
	}

	if (input->WasKeyJustPressed('O'))
	{
		m_cFunctionSubscribed = !m_cFunctionSubscribed;

		if (m_cFunctionSubscribed)
		{
			eventSystem->SubscribeEventCallbackFunction("Test", EventSystemCCallback);
		}
		else
		{
			eventSystem->UnsubscribeEventCallbackFunction("Test", EventSystemCCallback);
		}
	}

	// Toggling consumption
	if (input->WasKeyJustPressed('J'))
	{
		m_staticFunctionShouldConsume = !m_staticFunctionShouldConsume;
	}

	if (input->WasKeyJustPressed('K'))
	{
		m_objectMethodShouldConsume = !m_objectMethodShouldConsume;
	}

	if (input->WasKeyJustPressed('L'))
	{
		m_cFunctionShouldConsume = !m_cFunctionShouldConsume;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input for testing the job system
//
void Game::ProcessJobSystemInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->WasKeyJustPressed('B'))
	{
		std::string nextID = Stringf("Thread%i", m_workerThreadIDs.size());
		m_workerThreadIDs.push_back(nextID);

		JobSystem::GetInstance()->CreateWorkerThread(nextID.c_str(), WORKER_FLAGS_ALL_BUT_DISK);
	}

	if (input->WasKeyJustPressed('V'))
	{
		if (m_workerThreadIDs.size() > 0)
		{
			std::string idToRemove = m_workerThreadIDs[m_workerThreadIDs.size() - 1];
			m_workerThreadIDs.pop_back();

			JobSystem::GetInstance()->DestroyWorkerThread(idToRemove.c_str());
		}
	}

	// No pushing more jobs until the previous are done >.>
	if (m_numJobsFinished < m_totalCreatedJobs)
	{
		return;
	}


	if (input->WasKeyJustPressed('N'))
	{
		// Push a bunch of jobs to the job system
		for (int i = 0; i < 1000; i++)
		{
			CountJob* countJob = new CountJob();
			QueueJob(countJob);
		}
	}

	if (input->WasKeyJustPressed('M'))
	{
		// Run a bunch of jobs right now on the main thread
		for (int i = 0; i < 1000; i++)
		{
			CountJob* countJob = new CountJob();

			countJob->Execute();
			countJob->Finalize();
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Static event for testing
//
bool Game::EventSystemStaticCallback(NamedProperties& args)
{
	if (s_instance->m_staticFunctionShouldConsume)
	{
		s_instance->m_eventResultsText += "    -STATIC: Fired AND Consumed\n";
	}
	else
	{
		s_instance->m_eventResultsText += "    -STATIC: Fired (Did NOT consume)\n";
	}

	return s_instance->m_staticFunctionShouldConsume;
}


//-----------------------------------------------------------------------------------------------
// Object Method callback for testing
//
bool Game::EventSystemObjectMethodCallback(NamedProperties& args)
{
	if (m_objectMethodShouldConsume)
	{
		m_eventResultsText += "    -OBJECT METHOD: Fired AND Consumed\n";
	}
	else
	{
		m_eventResultsText += "    -OBJECT METHOD: Fired (Did NOT consume)\n";
	}

	return m_objectMethodShouldConsume;
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

	DebugRenderOptions options;
	options.m_lifetime = 99999.f;
	options.m_renderMode = DEBUG_RENDER_XRAY;

	DebugRenderSystem::DrawBasis(Matrix44::IDENTITY, options);
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
	InputSystem* input = InputSystem::GetInstance();
	Mouse& mouse = InputSystem::GetMouse();

	float deltaTime = Game::GetDeltaTime();
	IntVector2 delta = mouse.GetMouseDelta();

	// Translating the camera
	Vector3 translationOffset = Vector3::ZERO;
	if (input->IsKeyPressed('W')) { translationOffset.x += 1.f; }		// Forward
	if (input->IsKeyPressed('A')) { translationOffset.y += 1.f; }		// Left
	if (input->IsKeyPressed('S')) { translationOffset.x -= 1.f; }		// Back
	if (input->IsKeyPressed('D')) { translationOffset.y -= 1.f; }		// Right

	translationOffset = m_gameCamera->GetCameraMatrix().TransformVector(translationOffset).xyz();
	translationOffset.z = 0.f;
	translationOffset.NormalizeAndGetLength();

	if (input->IsKeyPressed('E')) { translationOffset.z += 1.f; }		// Up
	if (input->IsKeyPressed('Q')) { translationOffset.z -= 1.f; }		// Down

	if (input->IsKeyPressed(InputSystem::KEYBOARD_SHIFT))
	{
		translationOffset *= 8.f;
	}
	else if (input->IsKeyPressed(InputSystem::KEYBOARD_SPACEBAR))
	{
		translationOffset *= 0.25f;
	}


	translationOffset *= CAMERA_TRANSLATION_SPEED * deltaTime;

	m_gameCamera->TranslateWorld(translationOffset);

	Vector2 rotationOffset = Vector2((float)delta.y, (float)delta.x) * 0.12f;
	Vector3 rotation = Vector3(0.f, rotationOffset.x * CAMERA_ROTATION_SPEED * deltaTime, -1.0f * rotationOffset.y * CAMERA_ROTATION_SPEED * deltaTime);

	Vector3 cameraRotation = m_gameCamera->Rotate(rotation);

	// Clamp to avoid going upside-down
	cameraRotation.x = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.x);
	cameraRotation.y = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.y);
	cameraRotation.z = GetAngleBetweenMinusOneEightyAndOneEighty(cameraRotation.z);

	cameraRotation.y = ClampFloat(cameraRotation.y, -85.f, 85.f);
	m_gameCamera->SetRotation(cameraRotation);

	ProcessEventSystemInput();
	ProcessJobSystemInput();
}


//-----------------------------------------------------------------------------------------------
// Update the movement variables of every entity in the world, as well as update the game state
// based on the input this frame
//
void Game::Update()
{
	if (m_eventFiredTimer.HasIntervalElapsed())
	{
		m_eventResultsText.clear();
		m_eventFiredTimer.Reset();
	}

	JobSystem::GetInstance()->FinalizeAllFinishedJobsOfType(5); // Finalize the count jobs
}



//-----------------------------------------------------------------------------------------------
// Draws to screen
//
void Game::Render() const
{
	Window* window = Window::GetInstance();
	AABB2 bounds = window->GetWindowBounds();

	// Event system rendering
	std::string subscribeText = Stringf("-----EventSystem-----\nSubscriptions (Not Necessarily fired in this order):\n    -Static Function Subscribed [U]: %s | Consume Event[J]: %s\n    -Object Method Subscribed[I]: %s | Consume Event[K]: %s\n    -C Function Subscribed[O]: %s | Consume Event[L]: %s\n\n\n",
		(m_staticFunctionSubscribed ? "YES" : "NO"), (m_staticFunctionShouldConsume ? "YES" : "NO"), (m_objectMethodSubscribed ? "YES" : "NO"), (m_objectMethodShouldConsume ? "YES" : "NO"), 
		(m_cFunctionSubscribed ? "YES" : "NO"), (m_cFunctionShouldConsume ? "YES" : "NO"));

	DebugRenderSystem::Draw2DText(subscribeText, bounds, 0.f, Rgba::DARK_GREEN, 20.f);

	std::string results;
	
	if (!m_eventFiredTimer.HasIntervalElapsed())
	{
		results = Stringf("EventFired!\n\n%s\n\n(For testing, I added this 3 second pause before you can fire again\nfor readability of results)", m_eventResultsText.c_str());
	}
	else
	{
		results = "Next Event Ready! Press 'Y' to fire.";
	}

	bounds.maxs.y -= 100.f;
	DebugRenderSystem::Draw2DText(results, bounds, 0.f, Rgba::CYAN, 20.f, Vector2(0.f, 0.f));

	// Job System render
	std::string stateText = Stringf("-----JobSystem-----\nJobs used for testing each count to 1,000,000 then return\nStatus: %s\nJobs Completed: %i of %i\nWorker Thread Count [V,B]: %i", (m_numJobsFinished < m_totalCreatedJobs ? "Pending Job Completion - Please Wait" : "IDLE\nPress:\n    N - Create 1000 jobs for worker threads\n    M - Create 1000 jobs on main thread only (WILL STALL)"), m_numJobsFinished, m_totalCreatedJobs, m_workerThreadIDs.size());
	DebugRenderSystem::Draw2DText(stateText, bounds, 0.f, Rgba::PURPLE, 20.f, Vector2(0.0f, 1.0f));
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


//-----------------------------------------------------------------------------------------------
// C Function event for testing
//
bool EventSystemCCallback(NamedProperties& args)
{
	Game* game = Game::GetInstance();

	if (game->m_cFunctionShouldConsume)
	{
		game->m_eventResultsText += "    -C Function: Fired AND Consumed\n";
	}
	else
	{
		game->m_eventResultsText += "    -C Function: Fired (Did NOT consume)\n";
	}

	return game->m_cFunctionShouldConsume;
}
