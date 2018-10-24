#include "Game/Framework/PlayStates/PlayState_Defeat.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/World.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

PlayState_Defeat::PlayState_Defeat()
	: PlayState(DEFEAT_TRANSITION_IN_TIME, DEFEAT_TRANSITION_OUT_TIME)
{
}

PlayState_Defeat::~PlayState_Defeat()
{
}

void PlayState_Defeat::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	bool spacePressed = input->WasKeyJustPressed(' ');

	if (spacePressed)
	{
		Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		for (int i = 0; i < MAX_PLAYERS; ++i)
		{
			XboxController& controller = input->GetController(i);

			if (controller.WasButtonJustPressed(XBOX_BUTTON_START) || controller.WasButtonJustPressed(XBOX_BUTTON_A))
			{
				Game::GetInstance()->TransitionToGameState(new GameState_MainMenu());
			}
		}
	}
}

bool PlayState_Defeat::Enter()
{
	return m_transitionTimer.HasIntervalElapsed();
}

void PlayState_Defeat::Update()
{
	// Nothing!
}

bool PlayState_Defeat::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}

void PlayState_Defeat::Render_Enter() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Defeat Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Defeat::Render() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Defeat!: Press Start/Space/A to return"), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Defeat::Render_Leave() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Defeat Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}
