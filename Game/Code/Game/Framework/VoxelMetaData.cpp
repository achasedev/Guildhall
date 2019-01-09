/************************************************************************/
/* File: VoxelMetaData.cpp
/* Author: Andrew Chase
/* Date: January 8th, 2018
/* Description: Implementation of the VoxelMetaData class
/************************************************************************/
#include "Game/Framework/VoxelMetaData.hpp"

// Defines for certain flags in the data
#define CASTS_SHADOWS_FLAG (0x1)
#define RECEIVES_SHADOWS_FLAG (0x2);


//-----------------------------------------------------------------------------------------------
// Sets whether this voxel should cast a shadow
//
void VoxelMetaData::SetCastsShadows(bool castsShadows)
{
	uint8_t mask = CASTS_SHADOWS_FLAG;
	
	if (castsShadows)
	{
		m_flags |= mask;
	}
	else
	{
		mask = ~mask;
		m_flags &= mask;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets whether this voxel should receive a shadow from another shadowcasting voxel
//
void VoxelMetaData::SetReceivesShadows(bool receivesShadows)
{
	uint8_t mask = RECEIVES_SHADOWS_FLAG;

	if (receivesShadows)
	{
		m_flags |= mask;
	}
	else
	{
		mask = ~mask;
		m_flags &= mask;
	}
}
