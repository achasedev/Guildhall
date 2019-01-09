
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

	// Color data (?)
};