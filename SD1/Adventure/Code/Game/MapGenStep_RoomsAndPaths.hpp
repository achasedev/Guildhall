/************************************************************************/
/* File: MapGenStep_RoomsAndPaths.hpp
/* Author: Andrew Chase
/* Date: November 6th, 2017
/* Bugs: None
/* Description: (sub)class to represent a map manipulation method
/*				Adds rectangular rooms to the map and connects them with paths
/************************************************************************/
#pragma once
#include <vector>
#include "Game/MapGenStep.hpp"
#include "Engine/Math/AABB2.hpp"
#include "ThirdParty/tinyxml2/tinyxml2.h"

class TileDefinition;
class HeatMap;

class MapGenStep_RoomsAndPaths : public MapGenStep
{
public:
	//-----Public Methods-----

	MapGenStep_RoomsAndPaths( const tinyxml2::XMLElement& genStepXmlElement );
	virtual ~MapGenStep_RoomsAndPaths() override;

	virtual void Run(Map& mapToAugment) override;


private:
	//-----Private Methods-----

	void			RunRoomsAndPaths(Map& mapToAugment) const;

	void			FindRoomsInMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const;
	AABB2				GetRandomRoomBounds(int interiorWidth, int interiorHeight, const IntVector2& mapDimensions) const;
	bool				DoesRoomOverlapExistingRooms(const AABB2& roomBoundsToCheck, std::vector<AABB2>& foundRooms) const;

	void			AddRoomsAndPathsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const;
	HeatMap*			AddWallsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const;
	void				AddPathsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const;
	void				AddFloorsToMap(std::vector<AABB2>& foundRooms, Map& mapToAugment) const;


private:
	//-----Private Data-----

	const TileDefinition*		m_floorType			= nullptr;
	const TileDefinition*		m_wallType			= nullptr;
	const TileDefinition*		m_pathType	= nullptr;

	IntRange					m_roomWidth		= IntRange(2,2);
	IntRange					m_roomHeight	= IntRange(2,2);
	IntRange					m_roomCount		= IntRange(1,1);
	IntRange					m_numOverlaps	= IntRange(0,0);

	bool						m_loopPath		= true;

};
