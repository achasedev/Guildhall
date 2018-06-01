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
#include "Engine/Math/AABB2.hpp"

class Camera
{
public:
	//-----Public Methods-----
	
	Camera();																			// Default constructor

	void Update(float deltaTime);														// Updates to follow the player

	// Mutators
	void SetPlayerView(const IntVector2& worldDimensions);						// Sets the view on screen to be on the player
	void SetMapView(const IntVector2& worldDimensions);							// Sets the view on the screen to be the center of the map, with vertical zoom equal to map height
	void StartScreenShake(float durationSeconds, float magnitude);						// Tells the camera to do a screenshake for the given amount of time and starting at the given magnitude

	// Accessors/Producers
	AABB2 GetCameraViewBounds() const;
	bool IsInCameraView(const AABB2& boundsToCheck);


private:
	//-----Private Methods-----

	void ClampBoundsToDimensions(const IntVector2& worldDimensions);


private:
	//-----Private Data-----

	Vector2 m_position;									// Position of the camera in world coordinates
	float m_orientation;								// 2D angle orientation of the camera
	float m_viewHeight;									// Zoom - number of tiles the camera can see vertically at all times
	float m_clientAspect;								// Aspect of the screen we're rendering to
	AABB2 m_viewBounds;									// The current bounds of the camera's view
	
	float m_screenShakeTimer;
	float m_screenShakeMagnitude;

	static constexpr float DEFAULT_VIEW_HEIGHT = 9.f;	// Default zoom factor is 7 tiles high
};
