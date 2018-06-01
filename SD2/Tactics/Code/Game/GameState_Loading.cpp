/************************************************************************/
/* File: GameState_Loading.cpp
/* Author: Andrew Chase
/* Date: February 12th, 2018
/* Description: Implementation of the GameState_Loading class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/IsoSprite.hpp"
#include "Game/GameCommon.hpp"
#include "Game/IsoAnimation.hpp"
#include "Game/AnimationSet.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/GameState_Loading.hpp"
#include "Game/GameState_MainMenu.hpp"
#include "Game/BoardStateDefinition.hpp"
#include "Engine/Core/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
//-----------------------------------------------------------------------------------------------
// Necessary override imposed by the GameState base class, is unused
//
void GameState_Loading::ProcessInput()
{
}


//-----------------------------------------------------------------------------------------------
// Checks if the scene is rendered, and if so loads resources from disk
//
void GameState_Loading::Update()
{
	// If the scene is already drawn, then load the resources and transition
	if (m_isSceneRendered)
	{
		LoadResources();
		Game::TransitionToGameState(new GameState_MainMenu());
	}
	else
	{
		// We will hit Render() right after this, but since Render is const
		// we need to set the member true here for now
		m_isSceneRendered = true;
	}
}


//-----------------------------------------------------------------------------------------------
// Draws the loading screen
//
void GameState_Loading::Render() const
{
	// Draw the loading screen
	Renderer* renderer = Renderer::GetInstance();

	renderer->ClearScreen(Rgba::BLUE);
	renderer->ClearDepth();
	renderer->DisableDepth();
	renderer->SetCurrentCamera(renderer->GetUICamera());

	BitmapFont* font = renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	AABB2 loadingBounds = AABB2(Vector2(0.35f * Window::GetInstance()->GetWindowAspect() * Renderer::UI_ORTHO_HEIGHT, 0.3f * Renderer::UI_ORTHO_HEIGHT), Vector2(0.65f * Window::GetInstance()->GetWindowAspect() * Renderer::UI_ORTHO_HEIGHT, 0.7f * Renderer::UI_ORTHO_HEIGHT));
	renderer->DrawTextInBox2D("Loading Game...", loadingBounds, Vector2(0.5f, 0.5f), 50.f, TEXT_DRAW_OVERRUN, font);

	renderer->EnableDepth(COMPARE_LESS, true);
}


void GameState_Loading::Enter()
{
}


void GameState_Loading::Leave()
{
}


//-----------------------------------------------------------------------------------------------
// Loads external resources from disk using the Renderer
//
void GameState_Loading::LoadResources() const
{
	// Preload any game resources here
	Renderer* renderer = Renderer::GetInstance();

	renderer->CreateOrGetBitmapFont("SquirrelFixedFont");
 	renderer->CreateOrGetShaderProgram("Block");
	renderer->CreateOrGetTexture("Data/Images/BlockBorder.png");

	// Load all Sprites first
	SpriteSheet::LoadSpriteSheet("Data/XML/SpriteSheets/SpriteSheet_Archer_Female.xml");
	SpriteSheet::LoadSpriteSheet("Data/XML/SpriteSheets/SpriteSheet_Mage_Female.xml");
	SpriteSheet::LoadSpriteSheet("Data/XML/SpriteSheets/SpriteSheet_Knight_Female.xml");

	// "Collect" sprites together to make IsoSprites
	IsoSprite::LoadIsoSprites("Data/XML/IsoSprites/IsoSprite_Archer_Female.xml");
	IsoSprite::LoadIsoSprites("Data/XML/IsoSprites/IsoSprite_Mage_Female.xml");
	IsoSprite::LoadIsoSprites("Data/XML/IsoSprites/IsoSprite_Knight_Female.xml");

	// Construct IsoAnimations from the IsoSprites
	IsoAnimation::LoadIsoAnimations("Data/XML/IsoAnimations/IsoAnimation_Archer_Female.xml");
	IsoAnimation::LoadIsoAnimations("Data/XML/IsoAnimations/IsoAnimation_Mage_Female.xml");
	IsoAnimation::LoadIsoAnimations("Data/XML/IsoAnimations/IsoAnimation_Knight_Female.xml");

	// Construct the AnimationSets for the actors
	AnimationSet::LoadAnimationSet("Data/XML/AnimationSets/AnimationSet_Archer_Female.xml");
	AnimationSet::LoadAnimationSet("Data/XML/AnimationSets/AnimationSet_Mage_Female.xml");
	AnimationSet::LoadAnimationSet("Data/XML/AnimationSets/AnimationSet_Knight_Female.xml");

	// Load the actor after all actor information is in
	ActorDefinition::LoadDefinitions("Data/XML/Definitions/ActorDefinitions.xml");

	// Finally, load the possible board states
	BoardStateDefinition::LoadDefinitions("Data/XML/Definitions/BoardStateDefinitions.xml");
}
