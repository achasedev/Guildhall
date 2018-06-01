/************************************************************************/
/* File: Encounter.hpp
/* Author: Andrew Chase
/* Date: February 22nd, 2017
/* Description: Class to represent a map and set of encounters
/************************************************************************/
#pragma once
#include <string>
#include <vector>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"

class Map;
class Actor;
class Animator;
class IsoSprite;
class HeatMap;
class IntVector2;
class OrbitCamera;
class ActorDefinition;
class BoardStateDefinition;

class BoardState
{
public:

	BoardState(BoardStateDefinition* definition);
	~BoardState();

	void Update();
	void Render() const; // Unused at the moment, use the sub-render methods below
		void RenderMap() const;
		void RenderActors() const;

	// Map functions
	void InitializeMapFromDefinition();
	Map* GetMap() const;
	std::vector<IntVector3> GetTraversableCoords(const IntVector3& startBlock, int speed, int jump, int teamIndex, HeatMap*& out_distanceMap) const;
	std::vector<IntVector3> GetAttackableCoords(const IntVector3& startBlock) const;
	std::vector<IntVector3> GetCoordsWithinManhattanDistance(const IntVector3& startBlock, int minDistance, int maxDistance) const;

	IntVector2  GetMapUpRelativeToCamera() const;


	// Actor functions
	void			SpawnActorsFromDefinition();

	void			AddActor(const std::string& actorName, ActorDefinition* definition, const IntVector2& spawnCoords, unsigned int teamIndex);
	void			DestroyAllActors();

	unsigned int	GetActorCount();
	Actor*			GetActorByIndex(unsigned int actorIndex) const;
	Actor*			GetActorAtCoords(const IntVector2& mapCoords) const;

	void			DeleteActor(Actor* actor);
	void			DeleteDeadActors();

	void			DecrementAllActorWaitTimes(float decrementAmount);
	void			SortActorsByNextTurn();
	int				GetActorTurnOrder(Actor* actor) const;


private:
	//-----Private Methods-----

	void UpdateAllActors();
	

private:
	//-----Private Data-----

	Map* m_map;
	std::vector<Actor*> m_actors;

	BoardStateDefinition* m_definition;

};
