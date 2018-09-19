#pragma once
#include "Engine/Math/Vector3.hpp"

class Texture3D;


class Entity
{
public:
	//-----Public Methods-----

	Entity();
	virtual ~Entity();

	virtual void Update();

	Texture3D* GetTexture() const;


protected:
	//-----Protected Data-----

	Vector3		m_position;
	float		m_orientation;

	Texture3D*	m_texture; // 8x8x8

};
