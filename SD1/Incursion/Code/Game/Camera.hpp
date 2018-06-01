/************************************************************************/
/* Project: Incursion
/* File: Camera.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2017
/* Bugs: None
/* Description: Class that sets the view projection to be on the player
				with a given zoom
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"

// Different views the camera can take on
enum CameraView {
	CAMERA_VIEW_ERROR = -1, 
	CAMERA_VIEW_PLAYER,		// Follow the player
	CAMERA_VIEW_MAP,		// Center of the map, with full vertical dimension rendered
	NUM_CAMERA_VIEWS
};

class Camera
{
public:
	//-----Public Methods-----
	
	Camera();																			// Default constructor

	void Update(float deltaTime);														// Updates to follow the player
	void SetDefaultView() const;														// Sets the view on screen to be on the player

	void SetMapView(const IntVector2& worldDimensions) const;							// Sets the view on the screen to be the center of the map, with vertical zoom equal to map height
	void StartScreenShake(float durationSeconds, float magnitude);						// Tells the camera to do a screenshake for the given amount of time and starting at the given magnitude


private:
	//-----Private Data-----

	Vector2 m_worldPosition;							// Position of the camera
	float m_orientation;								// 2D angle oreientation of the camera
	float m_viewHeight;									// Zoom - number of tiles the camera can see vertically at all times
	CameraView m_viewMode;								// Current view mode of the camera

	float m_screenShakeTimer;
	float m_screenShakeMagnitude;

	static constexpr float DEFAULT_ZOOM_FACTOR = 7.f;	// Default zoom factor is 7 tiles high
};
