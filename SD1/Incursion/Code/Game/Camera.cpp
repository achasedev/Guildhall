/************************************************************************/
/* Project: Incursion
/* File: Camera.cpp
/* Author: Andrew Chase
/* Date: October 1st, 2017
/* Bugs: None
/* Description: Implementation of the Camera class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/PlayerTank.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs a basic camera, with default values
//
Camera::Camera()
	: m_worldPosition(Vector2::ZERO)
	, m_orientation(0.f)
	, m_viewHeight(DEFAULT_ZOOM_FACTOR)
	, m_screenShakeTimer(0.f)
	, m_screenShakeMagnitude(0.f)
{
}


//-----------------------------------------------------------------------------------------------
// Sets the camera to the players position, and resets the orientation to 0.f, and the zoom to default
//
void Camera::Update(float deltaTime)
{
	m_worldPosition = g_thePlayer->GetPosition();
	m_orientation = 0.f;
	m_viewHeight = DEFAULT_ZOOM_FACTOR;

	// Decrement the screen shake timer
	m_screenShakeTimer -= deltaTime;
	if (m_screenShakeTimer < 0.f)
	{
		m_screenShakeTimer = 0.f;
		m_screenShakeMagnitude = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the projection of the screen to be orthographic, and with the current zoom
//
void Camera::SetDefaultView() const
{
	float viewWidth = (CLIENT_ASPECT * m_viewHeight);
	Vector2 halfScreen = 0.5f * Vector2(viewWidth, m_viewHeight);
	Vector2 bottomLeft = (m_worldPosition - halfScreen);
	Vector2 topRight = (m_worldPosition + halfScreen);

	Vector2 screenShakeOffset = Vector2::ZERO;

	// If the game is over or paused, don't screenshake
	if (!g_theGame->IsPaused() && !g_theGame->IsGameOver())
	{
		screenShakeOffset = Vector2::GetRandomVector(m_screenShakeMagnitude);
	}

	AABB2 viewBounds = AABB2(bottomLeft + screenShakeOffset, topRight + screenShakeOffset);

	g_theRenderer->SetOrtho(viewBounds);
}


//-----------------------------------------------------------------------------------------------
// Sets the view on the screen to be the center of the map, with vertical zoom equal to map height
//
void Camera::SetMapView(const IntVector2& worldDimensions) const
{
	float viewHeight = static_cast<float>(worldDimensions.y);
	float viewWidth = (CLIENT_ASPECT * viewHeight);
	Vector2 halfScreen = 0.5f * Vector2(viewWidth, viewHeight);

	Vector2 mapCenter = (worldDimensions.GetAsFloats() * 0.5f);

	Vector2 bottomLeft = (mapCenter - halfScreen);
	Vector2 topRight = (mapCenter + halfScreen);

	AABB2 viewBounds = AABB2(bottomLeft, topRight);

	g_theRenderer->SetOrtho(viewBounds);
}


//-----------------------------------------------------------------------------------------------
// Tells the camera to perform screen shake in its update with the given duration and magnitude
//
void Camera::StartScreenShake(float durationSeconds, float magnitude)
{
	m_screenShakeTimer = durationSeconds;
	m_screenShakeMagnitude = magnitude;
}
