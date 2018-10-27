/************************************************************************/
/* File: MovingEntity.hpp
/* Author: Andrew Chase
/* Date: October 9th, 2018
/* Description: Class to represent an entity that can move itself
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

class AnimatedEntity : public Entity
{
public:
	//-----Public Methods-----
	
	AnimatedEntity(const EntityDefinition* definition);
	virtual ~AnimatedEntity();

	virtual void Update() override;
	virtual const VoxelTexture* GetTextureForRender() const override;

	// Events
	virtual void	OnCollision(Entity* other) override;
	virtual void	OnDamageTaken(int damageAmount) override;
	virtual void	OnDeath() override;
	virtual void	OnSpawn() override;
	

public:
	//-----Public Methods-----

	void Move(const Vector2& direction);
	void Jump();
	void Decelerate();


protected:
	//-----Protected Data-----

	// Animation
	VoxelAnimator*			m_animator = nullptr;

};
