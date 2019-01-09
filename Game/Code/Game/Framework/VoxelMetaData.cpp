#include "Game/Framework/VoxelMetaData.hpp"

#define CASTS_SHADOWS_FLAG (0x1)
#define RECEIVES_SHADOWS_FLAG (0x2);

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

