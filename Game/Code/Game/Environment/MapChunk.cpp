#include "Game/Framework/Game.hpp"
#include "Game/Environment/MapChunk.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"

MapChunk::MapChunk(Mesh* mesh, Material* material)
{
	m_renderable = new Renderable(Matrix44::IDENTITY, mesh, material);
	Game::GetRenderScene()->AddRenderable(m_renderable);
}

MapChunk::~MapChunk()
{
	TODO("Mesh needs to be deleted at some point");
	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
}

