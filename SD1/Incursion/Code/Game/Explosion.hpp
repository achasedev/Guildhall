/************************************************************************/
/* Project: Incursion
/* File: Explosion.hpp
/* Author: Andrew Chase
/* Date: October 15th, 2017
/* Bugs: None
/* Description: Class to represent an explosion sprite animation
/************************************************************************/
#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/SpriteAnimation.hpp"

class Explosion : public Entity
{
public:
	//-----Public Methods-----

	Explosion(Vector2 position, float radius, float duration, Map* theMap);		// Constructs similar to all other entity subclasses
	virtual ~Explosion() override;												// Destructor required to delete the sprite animation pointer allocation

	virtual void Update(float deltaTime) override;								// Update used to increment age and update the sprite animation object
	virtual void Render() const override;										// Render to draw the sprite animation


private:
	//-----Private Data-----

	SpriteAnimation* m_spriteAnimation;											// The sprite animation object itself, made from the 5x5 explosion sprite sheet
};