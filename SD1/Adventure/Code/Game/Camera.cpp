/************************************************************************/
/* File: Camera.cpp
/* Author: Andrew Chase
/* Date: November 29th, 2017
/* Bugs: None
/* Description: Implementation of the Camera class
/************************************************************************/
#include "Game/Game.hpp"
#include "Game/Camera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs a basic camera, with default values
//
Camera::Camera()
	: m_position(Vector2::ZERO)
	, m_orientation(0.f)
	, m_viewHeight(DEFAULT_VIEW_HEIGHT)
	, m_screenShakeTimer(0.f)
	, m_screenShakeMagnitude(0.f)
	, m_viewBounds(*g_screenBounds)
{
	Vector2 screenDimensions = g_screenBounds->GetDimensions();
	m_clientAspect = (screenDimensions.x / screenDimensions.y);
}


//-----------------------------------------------------------------------------------------------
// Update used for screenshake
//
void Camera::Update(float deltaTime)
{
	// Decrement the screen shake timer
	m_screenShakeTimer -= deltaTime;
	if (m_screenShakeTimer < 0.f)
	{
		m_screenShakeTimer = 0.f;
		m_screenShakeMagnitude = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the projection of the screen to be orthographic, and with the current zoom on the player
//
void Camera::SetPlayerView(const IntVector2& worldDimensions)
{
	// Determine the center position of the view
	m_position = g_thePlayer->GetPosition();

	// Determine the initial view bounds
	m_viewHeight = DEFAULT_VIEW_HEIGHT;
	float viewWidth = (m_clientAspect * m_viewHeight);
	Vector2 halfScreen = 0.5f * Vector2(viewWidth, m_viewHeight);

	Vector2 bottomLeft = (m_position - halfScreen);
	Vector2 topRight = (m_position + halfScreen);

	m_viewBounds = AABB2(bottomLeft, topRight);

	// Clamp the view to within the world bounds
	ClampBoundsToDimensions(worldDimensions);
	
	// Apply screen shake if the timer is active
	Vector2 screenShakeOffset = Vector2::ZERO;
	if (m_screenShakeTimer > 0.f)
	{
		screenShakeOffset = Vector2::GetRandomVector(m_screenShakeMagnitude);
	}

	// Translate the screenshake
	m_viewBounds.Translate(screenShakeOffset);

	// Set the ortho
	g_theRenderer->SetOrtho(m_viewBounds);
}


//-----------------------------------------------------------------------------------------------
// Sets the view on the screen to be the center of the map, with vertical zoom equal to map height
//
void Camera::SetMapView(const IntVector2& worldDimensions)
{
	m_viewHeight = static_cast<float>(worldDimensions.y);
	float viewWidth = (m_clientAspect * m_viewHeight);
	Vector2 halfScreen = 0.5f * Vector2(viewWidth, m_viewHeight);

	// Set the position to be the center of the map
	m_position = (Vector2(worldDimensions) * 0.5f);

	Vector2 bottomLeft = (m_position - halfScreen);
	Vector2 topRight = (m_position + halfScreen);

	m_viewBounds = AABB2(bottomLeft, topRight);

	g_theRenderer->SetOrtho(m_viewBounds);
}


//-----------------------------------------------------------------------------------------------
// Tells the camera to perform screen shake in its update with the given duration and magnitude
//
void Camera::StartScreenShake(float durationSeconds, float magnitude)
{
	m_screenShakeTimer = durationSeconds;
	m_screenShakeMagnitude = magnitude;
}


//-----------------------------------------------------------------------------------------------
// Returns the AABB2 that represents the camera's current view in the world
//
AABB2 Camera::GetCameraViewBounds() const
{
	return m_viewBounds;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the AABB2 is at least partially visible in the camera view, and false otherwise
//
bool Camera::IsInCameraView(const AABB2& boundsToCheck)
{
	return DoAABBsOverlap(m_viewBounds, boundsToCheck);
}


//-----------------------------------------------------------------------------------------------
// Adjusts the view bounds to clamp to the edges of the map
//
void Camera::ClampBoundsToDimensions(const IntVector2& worldDimensions)
{
	Vector2 bottomLeftTranslation = Vector2::ZERO;

	if (m_viewBounds.mins.x < 0.f) {bottomLeftTranslation.x = -m_viewBounds.mins.x; }
	if (m_viewBounds.mins.y < 0.f) {bottomLeftTranslation.y = -m_viewBounds.mins.y; }

	m_viewBounds.Translate(bottomLeftTranslation);

	Vector2 topRightTranslation = Vector2::ZERO;

	if (m_viewBounds.maxs.x > worldDimensions.x) {topRightTranslation.x = (worldDimensions.x - m_viewBounds.maxs.x); }
	if (m_viewBounds.maxs.y > worldDimensions.y) {topRightTranslation.y = (worldDimensions.y - m_viewBounds.maxs.y); }

	m_viewBounds.Translate(topRightTranslation);
}
