/************************************************************************/
/* File: Encounter.hpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Class to represent a map and set of encounters
/************************************************************************/
#pragma once
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"

class Map;
class Actor;
class IntVector2;
class IsoSprite;
class OrbitCamera;

class Encounter
{
public:

	Encounter();
	~Encounter();

	void Update(float deltaTime);
	void Render() const; // Unused at the moment, use the sub-render methods below
		void RenderMap() const;
		void RenderActors(OrbitCamera* orbitCamera) const;
		void RenderWaitTimeDebug() const;

	// Map functions
	void LoadMap(const char* filename);
	Map* GetMap() const;

	// Actor functions
	void			AddActor(unsigned int team, const std::string& actorName, const IntVector2& mapCoords, IsoSprite* sprite, const Rgba& color = Rgba::WHITE);
	void			DestroyAllActors();

	unsigned int	GetActorCount();
	Actor*			GetActorByIndex(unsigned int actorIndex) const;
	Actor*			GetActorAtCoords(const IntVector2& mapCoords) const;

	void			DeleteActor(Actor* actor);
	void			DeleteDeadActors();

	void			DecrementAllActorWaitTimes(float decrementAmount);
	

private:
	//-----Private Methods-----

	void UpdateAllActors(float deltaTime);


private:
	//-----Private Data-----

	Map* m_map;
	std::vector<Actor*> m_actors;
};
