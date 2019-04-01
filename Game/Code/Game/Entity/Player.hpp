/************************************************************************/
/* File: Player.hpp
/* Author: Andrew Chase
/* Date: April 1st 2019
/* Description: Class to represent the player entity
/************************************************************************/
#pragma once
#include "Game/Entity/Entity.hpp"

class Player : public Entity
{
public:
	//-----Public Methods-----

	void			ProcessInput();
	virtual void	Update() override;
	virtual void	Render() const override;


private:
	//-----Private Data-----


};
