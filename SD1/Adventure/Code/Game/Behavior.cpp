#include <string>
#include "Game/Behavior.hpp"
#include "Game/Behavior_Flee.hpp"
#include "Game/Behavior_Wander.hpp"
#include "Game/Behavior_RangedAttack.hpp"
#include "Game/Behavior_MeleeAttack.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtilities.hpp"
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Math/MathUtils.hpp"

std::map<std::string, Behavior*> Behavior::s_behaviorPrototypes;


Behavior::Behavior(const XMLElement& behaviorElement)
	: m_actor(nullptr)
{
	// Parse behavior stuff here
	m_name = behaviorElement.Name();
}


std::string Behavior::GetName() const
{
	return m_name;
}

void Behavior::SetActor(Actor* actor)
{
	m_actor = actor;
}

//-----------------------------------------------------------------------------------------------
// Loads the Behavior XML file and constructs all behavior prototypes
//
void Behavior::LoadAndCreatePrototypes()
{
	// Load the XML file, and parse the root-child elements for tile definitions
	tinyxml2::XMLDocument behaviorDocument;
	behaviorDocument.LoadFile("Data/Definitions/Behavior.xml");

	tinyxml2::XMLElement* currBehaviorElement = behaviorDocument.RootElement()->FirstChildElement();

	while (currBehaviorElement != nullptr)
	{
		Behavior* newBehavior = CreatePrototypeBehavior(*currBehaviorElement);

		// Safety check - no duplicate definitions
		bool definitionAlreadyMade = (s_behaviorPrototypes.find(newBehavior->GetName()) != s_behaviorPrototypes.end());
		GUARANTEE_OR_DIE(!definitionAlreadyMade, Stringf("Error: Duplicate behavior in Behavior.xml - \"%s\"", newBehavior->GetName().c_str()));

		// Add the definition to the map
		s_behaviorPrototypes[newBehavior->GetName()] = newBehavior;

		// Move to the next element
		currBehaviorElement = currBehaviorElement->NextSiblingElement();
	}
}

Behavior* Behavior::CreatePrototypeBehavior(const XMLElement& behaviorElement)
{
	std::string behaviorName = behaviorElement.Name();

	if		(behaviorName.compare("Flee") == 0) { return new Behavior_Flee(behaviorElement); }
	else if (behaviorName.compare("RangedAttack") == 0) { return new Behavior_RangedAttack(behaviorElement); }
	else if (behaviorName.compare("Wander") == 0) { return new Behavior_Wander(behaviorElement); }
	else if (behaviorName.compare("MeleeAttack") == 0) { return new Behavior_MeleeAttack(behaviorElement); }

	else { ERROR_AND_DIE(Stringf("Error: Couldn't create MapGenStep of subclass \"%s\"", behaviorName.c_str()));}
}



Behavior* Behavior::ClonePrototypeBehavior(const std::string& behaviorName)
{
	// Ensure the behavior exist
	bool behaviorExists = (s_behaviorPrototypes.find(behaviorName) != s_behaviorPrototypes.end());
	GUARANTEE_OR_DIE(behaviorExists, Stringf("Error: Behavior::ClonePrototypeBehavior could not clone behavior \"%s\" - it doesn't exist", behaviorName.c_str()));

	Behavior* prototype = s_behaviorPrototypes[behaviorName];

	return prototype->Clone();
}


void Behavior::UpdateNeighborDistance(const IntVector2& currCoords, const IntVector2& stepDirection, HeatMap* distanceMap, std::queue<IntVector2>& distanceQueue) const
{
	Map* actorMap = m_actor->GetMap();
	IntVector2 neighborCoords = currCoords + stepDirection;

	// Ensure coordinates are in the map boundary
	if (actorMap->AreCoordsValid(neighborCoords))
	{
		Tile* neighborTile = actorMap->GetTileFromCoords(neighborCoords);

		// Only update tiles this actor can travel into
		if (actorMap->EntityCanEnterTile(m_actor, neighborTile))
		{
			// Check to update the distance
			float existingNeighborDistance		= distanceMap->GetHeat(neighborCoords);
			float newNeighborDistance			= distanceMap->GetHeat(currCoords) + 1.f;

			if (newNeighborDistance < existingNeighborDistance)
			{
				distanceMap->SetHeat(neighborCoords, newNeighborDistance);
				distanceQueue.push(neighborCoords);
			}
		}
	}
}

HeatMap* Behavior::ConstructDistanceMapToTilePosition(const IntVector2& targetTileCoords) const
{
	// Initialize the heat map - 9999 everywhere except 0 on the target tile
	HeatMap* distanceMap = new HeatMap(m_actor->GetMap()->GetDimensions(), 9999.f);
	distanceMap->SetHeat(targetTileCoords, 0.f);

	// Create the queue and place the target in it
	std::queue<IntVector2> distanceQueue;
	distanceQueue.push(targetTileCoords);

	// Iterate until there are no more updates
	while (static_cast<int>(distanceQueue.size()) > 0)
	{
		IntVector2 currTileCoords = distanceQueue.front();
		distanceQueue.pop();

		// Update the neighbors
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_NORTH,	distanceMap, distanceQueue);	// North
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_SOUTH,	distanceMap, distanceQueue);	// South
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_EAST,	distanceMap, distanceQueue);	// East
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_WEST,	distanceMap, distanceQueue);	// West
	}

	return distanceMap;
}


IntVector2 Behavior::GetMinNeighborCoords(HeatMap* distanceMap, const IntVector2& currCoords) const
{
	// Get the distances of the neighbors

	IntVector2 northCoords	= currCoords + IntVector2::STEP_NORTH;
	IntVector2 southCoords	= currCoords + IntVector2::STEP_SOUTH;
	IntVector2 eastCoords	= currCoords + IntVector2::STEP_EAST;
	IntVector2 westCoords	= currCoords + IntVector2::STEP_WEST;

	// Ensure we only check coords that are in bounds
	float northDistance = (distanceMap->AreCoordsInBounds(northCoords) ? distanceMap->GetHeat(northCoords) : -9999.f);
	float southDistance = (distanceMap->AreCoordsInBounds(southCoords) ? distanceMap->GetHeat(southCoords) : -9999.f);
	float eastDistance	= (distanceMap->AreCoordsInBounds(eastCoords) ? distanceMap->GetHeat(eastCoords) : -9999.f);
	float westDistance	= (distanceMap->AreCoordsInBounds(westCoords) ? distanceMap->GetHeat(westCoords) : -9999.f);

	float minDistance = MinFloat(northDistance, southDistance, eastDistance, westDistance);

	// Return the appropriate coords
	if	(minDistance == eastDistance) { return eastCoords; }
	else if (minDistance == westDistance) { return westCoords; }
	else if (minDistance == northDistance) { return northCoords; }
	else { return southCoords; }
}

void Behavior::SetActorVelocityAlongPath(const IntVector2& endCoords) const
{
	// Generate a distance map used for navigation
	HeatMap* distanceMap = ConstructDistanceMapToTilePosition(endCoords);
	Vector2 actorPos = m_actor->GetPosition();
	IntVector2 actorCoords = IntVector2(actorPos.x, actorPos.y);

	// Get the min neighbor coords
	IntVector2 nextCoordsInPath = GetMinNeighborCoords(distanceMap, actorCoords);

	// Next position is the center of the lowest neighbor tile
	Vector2 nextPositionInPath = Vector2(nextCoordsInPath) + Vector2(0.5f, 0.5f);

	// Set the velocity so the actor moves along the path
	float walkSpeed = m_actor->GetWalkSpeed();

	Vector2 displacement = nextPositionInPath - actorPos;
	Vector2 direction = displacement.GetNormalized();

	m_actor->SetOrientationDegrees(direction.GetOrientationDegrees());
	m_actor->SetVelocity(direction * walkSpeed);

	// Free the distance map
	delete distanceMap;
}


bool Behavior::AreTargetCoordsAccessible(const IntVector2& targetCoords) const
{
	HeatMap* distanceMap = ConstructDistanceMapToTilePosition(targetCoords);
	IntVector2 currCoords = IntVector2(static_cast<int>(m_actor->GetPosition().x), static_cast<int>(m_actor->GetPosition().y));

	bool isValid = false;

	if		(distanceMap->GetHeat(currCoords + IntVector2::STEP_NORTH) != 9999.f) { isValid = true; }
	else if (distanceMap->GetHeat(currCoords + IntVector2::STEP_SOUTH) != 9999.f) { isValid = true; }
	else if (distanceMap->GetHeat(currCoords + IntVector2::STEP_EAST) != 9999.f) { isValid = true; }
	else if (distanceMap->GetHeat(currCoords + IntVector2::STEP_WEST) != 9999.f) { isValid = true; }

	return isValid;
}