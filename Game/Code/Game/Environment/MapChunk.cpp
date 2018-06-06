/************************************************************************/
/* File: MapChunk.cpp
/* Author: Andrew Chase
/* Date: June 3rd, 2018
/* Description: Implementation of the MapChunk class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Environment/MapChunk.hpp"
#include "Game/Framework/GameCommon.hpp"

#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
MapChunk::MapChunk(const Matrix44& model, Mesh* mesh, Material* material)
{
	m_renderable = new Renderable(model, mesh, material);
	Game::GetRenderScene()->AddRenderable(m_renderable);
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
MapChunk::~MapChunk()
{
	TODO("Mesh needs to be deleted at some point");
	Game::GetRenderScene()->RemoveRenderable(m_renderable);
	delete m_renderable;
}
