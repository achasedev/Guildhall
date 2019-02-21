/************************************************************************/
/* File: GameCommon.cpp
/* Author: Andrew Chase
/* Date: March 24th, 2018
/* Description: Header file to be included in all game .cpp files
/************************************************************************/
#include "Engine/Math/MathUtils.hpp"
#include "Game/Framework/GameCommon.hpp"

//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the IntVector3 of position with all values floored
//
IntVector3 FloorPositionToIntegerCoords(const Vector3& position)
{
	int x = Floor(position.x);
	int y = Floor(position.y);
	int z = Floor(position.z);

	return IntVector3(x, y, z);
}
