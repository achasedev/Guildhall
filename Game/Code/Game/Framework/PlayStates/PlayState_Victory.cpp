#include "Game/Framework/PlayStates/PlayState_Victory.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"
#include "Game/Framework/World.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderSystem.hpp"

PlayState_Victory::PlayState_Victory()
	: PlayState(VICTORY_TRANSITION_IN_TIME, VICTORY_TRANSITION_OUT_TIME)
{
}

PlayState_Victory::~PlayState_Victory()
{
}

void PlayState_Victory::ProcessInput()
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

bool PlayState_Victory::Enter()
{
	return m_transitionTimer.HasIntervalElapsed();
}

void PlayState_Victory::Update()
{
	// Nothing!
}

bool PlayState_Victory::Leave()
{
	return m_transitionTimer.HasIntervalElapsed();
}

void PlayState_Victory::Render_Enter() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Victory Enter: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Victory::Render() const
{
	Game::GetWorld()->Render();
	DebugRenderSystem::Draw2DText(Stringf("Victory!: Press Start/Space/A to return"), Window::GetInstance()->GetWindowBounds(), 0.f);
}

void PlayState_Victory::Render_Leave() const
{
	Game::GetWorld()->Render();

	DebugRenderSystem::Draw2DText(Stringf("Victory Leave: %.2f seconds remaining", m_transitionTimer.GetTimeUntilIntervalEnds()), Window::GetInstance()->GetWindowBounds(), 0.f);
}

