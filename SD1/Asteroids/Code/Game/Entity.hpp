/************************************************************************/
/* Project: SD1 C27 A2 - Asteroids
/* File: Entity.hpp
/* Author: Andrew Chase
/* Date: September 7th, 2017
/* Bugs: None
/* Description: Base class for all spawn-able and rendered objects
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"


class Entity
{

public:

	void Update(float deltaTime);

	//-----Public Methods-----
	void CheckWrapAround(const AABB2& worldBounds);	// Checks if the Entity will wrap around the screen
	void DrawDebugInfo() const;							// Draws this Entity's Debug information

	//-----Mutators-----
	void SetPosition(const Vector2& newPosition);
	void SetVelocity(const Vector2& newVelocity);
	void SetOrientationDegrees(float newOrientationDegress);
	void SetAngularVelocity(float newAngularVelocity);
	void SetInnerRadius(float newInnerRadius);
	void SetOuterRadius(float newOuterRadius);

	//-----Accessors-----
	Vector2 GetPosition() const;
	Vector2 GetVelocity() const;
	float GetOrientationDegrees() const;
	float GetAngularVelocity() const;
	float GetInnerRadius() const;
	float GetOuterRadius() const;


protected:

	//-----Protected Data-----
	Vector2 m_position;				// Position in 2D space
	Vector2 m_velocity;				// Velocity in OpenGL-world units
	float m_orientationDegrees;		// Orientation angle of the object (on unit circle)
	float m_angularVelocity;		// Rotation velocity of the object
	float m_innerRadius;			// Physics radius of the object
	float m_outerRadius;			// Cosmetic radius of the object

	float m_age;					// Age of the entity, incremented every frame by deltaSeconds
	float m_ageAtDeath;				// Age of the entity at destruction
};
