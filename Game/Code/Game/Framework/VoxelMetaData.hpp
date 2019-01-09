/************************************************************************/
/* File: VoxelMetaData.hpp
/* Author: Andrew Chase
/* Date: January 8th, 2018
/* Description: Class to represent meta data for a single voxel in a grid
/************************************************************************/
#include <stdint.h>

class VoxelMetaData
{

public:
	//-----Public Methods-----

	void SetCastsShadows(bool castsShadows);
	void SetReceivesShadows(bool receivesShadows);


private:
	//-----Private Data-----

	int8_t m_flags = 0;

};
