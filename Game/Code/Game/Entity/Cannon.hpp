#pragma once
#include "Engine/Core/GameObject.hpp"

class Cannon : public GameObject
{
public:
	//-----Public Methods-----

	Cannon(Transform& parent);
	~Cannon();

	virtual void Update(float deltaTime) override;


private:
	//-----Private Data-----

};
