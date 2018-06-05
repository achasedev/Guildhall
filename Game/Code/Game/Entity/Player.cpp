#include "Game/Entity/Player.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Environment/Map.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/OrbitCamera.hpp"

Player::Player()
{
	Renderer* renderer = Renderer::GetInstance();

	m_camera = new OrbitCamera();
	m_camera->SetColorTarget(renderer->GetDefaultColorTarget());
	m_camera->SetDepthTarget(renderer->GetDefaultDepthTarget());
	m_camera->SetTarget(m_position);
	m_camera->SetRadius(10.f);
	m_camera->SetAzimuth(45.f);
}

void Player::ProcessInput()
{

}

void Player::Update(float deltaTime)
{
	GameObject::Update(deltaTime);

	UpdateHeightOnMap();
}

void Player::UpdateCameraOnInput()
{
	
}

void Player::UpdatePositionOnInput()
{

}

void Player::UpdateHeightOnMap()
{
	Map* map = Game::GetMap();
	float height = map->GetHeightAtPosition(m_position);
	m_position.y = height;
}

