#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Environment/Terrain.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Core/ForwardRenderingPath.hpp"

World::World()
{

}

void World::Inititalize()
{
	m_terrain = new Terrain();
	m_terrain->LoadFromFile("Data/Environment/Terrain.png");

	m_renderScene = new RenderScene("World");
	m_renderScene->AddCamera(Game::GetGameCamera());
	m_renderScene->AddRenderable(m_terrain->GetRenderable());
}

void World::Render() const
{
	ForwardRenderingPath::Render(m_renderScene);
}
