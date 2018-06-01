#pragma once
#include <queue>
#include <map>
#include "Engine/Math/IntVector2.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

typedef tinyxml2::XMLElement XMLElement;
class HeatMap;
class Actor;


class Behavior
{
public:
	//-----Public Methods-----

	Behavior(const XMLElement& behaviorElement);

	virtual void		Update(float deltaTime) = 0;	// Pure virtual
	virtual float		CalcUtility() = 0;				// Pure virtual

	virtual Behavior*	Clone() const = 0;				// Pure virtual	

	// Accessors
	std::string			GetName() const;

	// Mutators
	void				SetActor(Actor* actor);

	// static functions
	static void			LoadAndCreatePrototypes();
	static Behavior*	CreatePrototypeBehavior(const XMLElement& behaviorElement);
	static Behavior*	ClonePrototypeBehavior(const std::string& behaviorName);

private:
	//-----Private Methods-----

	void				UpdateNeighborDistance(const IntVector2& currCoords, const IntVector2& stepDirection, HeatMap* distanceMap, std::queue<IntVector2>& distanceQueue) const;


protected:
	//-----Protected Methods-----

	HeatMap*			ConstructDistanceMapToTilePosition(const IntVector2& targetTileCoords) const;
	bool				AreTargetCoordsAccessible(const IntVector2& targetCoords) const;
	IntVector2			GetMinNeighborCoords(HeatMap* distanceMap, const IntVector2& currCoords) const;
	void				SetActorVelocityAlongPath(const IntVector2& endCoords) const;


protected:

	std::string m_name;		// Name of the behavior
	Actor* m_actor;			// The actor that possesses this behavior

	Actor* m_target;		// The target of this entity

private:

	static std::map<std::string, Behavior*> s_behaviorPrototypes;	// The prototype behaviors for all actors to copy from

};
