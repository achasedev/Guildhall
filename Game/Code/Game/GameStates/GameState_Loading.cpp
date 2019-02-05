/************************************************************************/
/* File: GameState_Loading.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Implementation of the GameState_Loading class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/VoxelMap.hpp"
#include "Game/Framework/VoxelFont.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Game/Animation/VoxelSprite.hpp"
#include "Game/Framework/VoxelTerrain.hpp"
#include "Game/Framework/MapDefinition.hpp"
#include "Game/Entity/EntityDefinition.hpp"
#include "Game/Animation/VoxelAnimation.hpp"
#include "Game/Animation/VoxelAnimationSet.hpp"
#include "Game/GameStates/GameState_Loading.hpp"
#include "Game/Framework/CampaignDefinition.hpp"
#include "Game/GameStates/GameState_MainMenu.hpp"

#include "Engine/Core/Window.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Resources/Skybox.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/Material.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
GameState_Loading::GameState_Loading()
	: GameState(0.f, 0.f)
{
}


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
	renderer->SetCurrentCamera(renderer->GetUICamera());

	BitmapFont* font = AssetDB::CreateOrGetBitmapFont("Data/Images/Fonts/Default.png");
	AABB2 loadingBounds = AABB2(Vector2(0.35f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.3f * Renderer::UI_ORTHO_HEIGHT), Vector2(0.65f * Window::GetInstance()->GetAspect() * Renderer::UI_ORTHO_HEIGHT, 0.7f * Renderer::UI_ORTHO_HEIGHT));
	renderer->DrawTextInBox2D("Loading Game\n(with one second sleep)...", loadingBounds, Vector2(0.5f, 0.5f), 50.f, TEXT_DRAW_OVERRUN, font);
}


//-----------------------------------------------------------------------------------------------
// Renders the leave state of the state
//
void GameState_Loading::Render_Leave() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Enter updating step
//
bool GameState_Loading::Enter()
{
	return true;
}


//-----------------------------------------------------------------------------------------------
// Leave updating step
//
bool GameState_Loading::Leave()
{
	Game::GetInstance()->m_doneLoading = true;
	return true;
}


void GameState_Loading::Render_Enter() const
{
	Render();
}


//-----------------------------------------------------------------------------------------------
// Loads external resources from disk using the Renderer
//
void GameState_Loading::LoadResources() const
{
	LoadVoxelResources();
}


//-----------------------------------------------------------------------------------------------
// Loads all voxel assets used for rendering
//
void GameState_Loading::LoadVoxelResources() const
{
	XMLDocument document;
	XMLError error = document.LoadFile("Data/GameAssetsToLoad.xml");
	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, "Error: Game couldn't open VoxelAssets.xml file");

	const XMLElement* rootElement = document.RootElement();
	ASSERT_OR_DIE(rootElement != nullptr, "Error: VoxelAssets.xml has no root element.");

	// Voxel Sprites
	{
		const XMLElement* spritesElement = rootElement->FirstChildElement("VoxelSprites");
		ASSERT_OR_DIE(spritesElement != nullptr, "Error: VoxelAssets.xml has no VoxelSprites element.");

		const XMLElement* currSpriteElement = spritesElement->FirstChildElement();

		while (currSpriteElement != nullptr)
		{
			std::string filename = ParseXmlAttribute(*currSpriteElement, "file");
			ASSERT_OR_DIE(filename.size() > 0, "Error: VoxelAssets.xml has sprite element with no filename specified");

			VoxelSprite::LoadSpriteFile(filename);

			currSpriteElement = currSpriteElement->NextSiblingElement();
		}
	}


	// Voxel Animation Sets
	{
		const XMLElement* setsElement = rootElement->FirstChildElement("VoxelAnimationSets");
		ASSERT_OR_DIE(setsElement != nullptr, "Error: VoxelAssets.xml has no VoxelAnimationSets element.");

		const XMLElement* currSetElement = setsElement->FirstChildElement();

		while (currSetElement != nullptr)
		{
			std::string filename = ParseXmlAttribute(*currSetElement, "file");
			ASSERT_OR_DIE(filename.size() > 0, "Error: VoxelAssets.xml has animation set element with no filename specified");

			VoxelAnimationSet::LoadSet(filename);

			currSetElement = currSetElement->NextSiblingElement();
		}
	}


	// Voxel Animations
	{
		const XMLElement* animsElement = rootElement->FirstChildElement("VoxelAnimations");
		ASSERT_OR_DIE(animsElement != nullptr, "Error: VoxelAssets.xml has no VoxelAnimationSets element.");

		const XMLElement* currAnimElement = animsElement->FirstChildElement();

		while (currAnimElement != nullptr)
		{
			std::string filename = ParseXmlAttribute(*currAnimElement, "file");
			ASSERT_OR_DIE(filename.size() > 0, "Error: VoxelAssets.xml has animation element with no filename specified");

			VoxelAnimation::LoadVoxelAnimations(filename);

			currAnimElement = currAnimElement->NextSiblingElement();
		}
	}

	// Entity Definitions
	{
		const XMLElement* defsElement = rootElement->FirstChildElement("EntityDefinitions");
		GUARANTEE_OR_DIE(defsElement != nullptr, "No EntityDefinitions group element in GameAssetsToLoad.xml");

		if (defsElement != nullptr)
		{
			const XMLElement* defElement = defsElement->FirstChildElement("EntityDefinition");
			GUARANTEE_OR_DIE(defElement != nullptr, "No EntityDefinition Element in GameAssetsToLoad.xml");

			while (defElement != nullptr)
			{
				std::string defFilename = ParseXmlAttribute(*defElement, "file", "");
				GUARANTEE_OR_DIE(!IsStringNullOrEmpty(defFilename), "EntityDefinition element has no file specified in GameAssetsToLoad.xml");

				EntityDefinition::LoadDefinitions(defFilename);

				defElement = defElement->NextSiblingElement("EntityDefinition");
			}	
		}
	}

	// Voxel Terrains
	{
		const XMLElement* terrainsElement = rootElement->FirstChildElement("VoxelTerrains");
		GUARANTEE_OR_DIE(terrainsElement != nullptr, Stringf("No terrains specified in GameAssetsToLoad.xml"));

		const XMLElement* terrainElement = terrainsElement->FirstChildElement("VoxelTerrain");
		GUARANTEE_OR_DIE(terrainElement != nullptr, Stringf("No terrains specified in GameAssetsToLoad.xml"));

		while (terrainElement != nullptr)
		{
			std::string terrainFilePath = ParseXmlAttribute(*terrainElement, "file", "");
			GUARANTEE_OR_DIE(!IsStringNullOrEmpty(terrainFilePath), Stringf("Terrain element in GameAssetsToLoad.xml has an empty file path"));

			VoxelTerrain::LoadTerrainFile(terrainFilePath);

			terrainElement = terrainElement->NextSiblingElement("VoxelTerrain");
		}
	}

	// Maps
	{
		const XMLElement* mapsElement = rootElement->FirstChildElement("Maps");
		ASSERT_OR_DIE(mapsElement != nullptr, "Error: GameAssetsToLoad.xml has no maps");

		const XMLElement* mapElement = mapsElement->FirstChildElement("Map");

		while (mapElement != nullptr)
		{
			std::string mapFilePath = ParseXmlAttribute(*mapElement, "file", "");
			MapDefinition::LoadMap(mapFilePath);

			mapElement = mapElement->NextSiblingElement("Map");
		}
	}

	// Campaigns
	{
		const XMLElement* campaignsElement = rootElement->FirstChildElement("Campaigns");
		ASSERT_OR_DIE(campaignsElement != nullptr, "Error: GameAssetsToLoad.xml has no Campaigns group element");

		const XMLElement* campaignElement = campaignsElement->FirstChildElement("Campaign");
		ASSERT_OR_DIE(campaignElement != nullptr, "Error: GameAssetsToLoad.xml has no Campaigns specified");

		while (campaignElement != nullptr)
		{
			std::string campaignFilePath = ParseXmlAttribute(*campaignElement, "file", "");
			CampaignDefinition::LoadCampaign(campaignFilePath);

			campaignElement = campaignElement->NextSiblingElement("Map");
		}
	}
}
