#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Rendering/Resources/Texture3D.hpp"

enum FACING_DIRECTION
{
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST, 
	NUM_DIRECTIONS
};

class Entity
{
public:
	//-----Public Methods-----

	Entity();
	virtual ~Entity();

	virtual void Update();

	Texture3D* GetTextureForOrientation() const;


private:
	//-----Private Methods-----



protected:
	//-----Protected Data-----

	Vector3		m_position;
	float		m_orientation;

	Texture3D*	m_textures[NUM_DIRECTIONS];

};
