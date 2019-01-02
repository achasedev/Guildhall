#include "Game/Animation/VoxelSprite.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

// Registry for all sprites used in animations/cloned from for destructibles
std::map<std::string, const VoxelSprite*> VoxelSprite::s_sprites;


VoxelSprite::VoxelSprite()
{
}

VoxelSprite::~VoxelSprite()
{
	if (m_colorData != nullptr)
	{
		free(m_colorData);
		m_colorData = nullptr;
	}

	if (m_collisionFlags != nullptr)
	{
		free(m_collisionFlags);
		m_collisionFlags = nullptr;
	}
}

bool VoxelSprite::CreateFromFile(const char* filename, bool createCollisionMatrix)
{
	File* file = new File();
	bool opened = file->Open(filename, "r");

	if (!opened)
	{
		return false;
	}

	file->LoadFileToMemory();

	std::string currLine;

	// "Qubicle Exchange Format"
	file->GetNextLine(currLine);

	if (currLine != "Qubicle Exchange Format")
	{
		ERROR_RECOVERABLE(Stringf("Error: VoxelSprite::CreateFromFile() only supports QEF files"));
		return false;
	}

	// Version number
	file->GetNextLine(currLine);

	// Website
	file->GetNextLine(currLine);

	// Dimensions
	file->GetNextLine(currLine);
	
	if (!SetFromText(currLine, m_dimensions))
	{
		ERROR_RECOVERABLE(Stringf("Error: VoxelSprite::CreateFromFile() couldn't get the dimensions of the texture."));
		return false;
	}

	if (m_dimensions.x > MAX_TEXTURE_VOXEL_WIDTH)
	{
		createCollisionMatrix = false;
	}

	// Number of colors
	file->GetNextLine(currLine);

	int numColors;
	SetFromText(currLine, numColors);

	Rgba* colorPallette = (Rgba*)malloc(sizeof(Rgba) * numColors);
	memset(colorPallette, 0, sizeof(Rgba) * numColors);

	// Get the colors
	for (int i = 0; i < numColors; ++i)
	{
		file->GetNextLine(currLine);
		SetFromText(currLine, colorPallette[i]);
	}

	// Set up the texture colors
	if (m_colorData != nullptr)
	{
		free(m_colorData);
	}

	unsigned int voxelCount = m_dimensions.x * m_dimensions.y * m_dimensions.z;
	m_colorData = (Rgba*)malloc(sizeof(Rgba) * voxelCount);
	memset(m_colorData, 0, sizeof(Rgba) * voxelCount);

	// Set up the collision flags as well
	if (createCollisionMatrix)
	{
		int amount = sizeof(uint32_t) * m_dimensions.y * m_dimensions.z;
		m_collisionFlags = (uint32_t*)malloc(amount);
		memset(m_collisionFlags, 0, amount);
	}

	// Now get all the voxel colors
	while (!file->IsAtEndOfFile())
	{
		file->GetNextLine(currLine);

		if (currLine.size() == 0)
		{
			break;
		}

		std::vector<std::string> voxelTokens = Tokenize(currLine, ' ');

		// Get the voxel coords
		int xCoord = StringToInt(voxelTokens[0]);

		// *Flip from right handed to left handed basis
		xCoord = m_dimensions.x - xCoord - 1;

		int yCoord = StringToInt(voxelTokens[1]);
		int zCoord = StringToInt(voxelTokens[2]);

		int index = yCoord * (m_dimensions.x * m_dimensions.z) + zCoord * m_dimensions.x + xCoord;

		int colorIndex = StringToInt(voxelTokens[3]);

		m_colorData[index] = colorPallette[colorIndex];

		// Update the collision bit for this voxel
		if (createCollisionMatrix && m_colorData[index].a != 0)
		{
			uint32_t& flags = m_collisionFlags[yCoord * m_dimensions.z + zCoord];
			flags |= (TEXTURE_LEFTMOST_COLLISION_BIT >> xCoord);
		}
	}

	// Done!
	free(colorPallette);

	return true;
}

bool VoxelSprite::CreateFromColorStream(const Rgba* colors, const IntVector3& dimensions, bool createCollisionMatrix)
{
	m_dimensions = dimensions;

	if (m_colorData != nullptr)
	{
		free(m_colorData);
	}

	int numVoxels = dimensions.x * dimensions.y * dimensions.z;

	m_colorData = (Rgba*)malloc(sizeof(Rgba) * numVoxels);
	memcpy(m_colorData, colors, numVoxels * sizeof(Rgba));

	if (createCollisionMatrix)
	{
		m_collisionFlags = (uint32_t*)malloc(sizeof(uint32_t) * dimensions.y * dimensions.z);

		for (int y = 0; y < m_dimensions.y; ++y)
		{
			for (int z = 0; z < m_dimensions.z; ++z)
			{
				for (int x = 0; x < m_dimensions.x; ++x)
				{
					Rgba color = GetColorAtRelativeCoords(IntVector3(x, y, z), 0.f);

					if (color.a != 0)
					{
						int index = y * (m_dimensions.x * m_dimensions.z) + z * m_dimensions.x + x;
						int byteIndex = index / 8;
						int bitOffset = 7 - (index % 8);

						uint32_t& collisionByte = m_collisionFlags[byteIndex];
						collisionByte |= (1 << bitOffset);
					}
				}
			}
		}
	}

	return true;
}

VoxelSprite* VoxelSprite::Clone() const
{
	VoxelSprite* newTexture = new VoxelSprite();
	newTexture->m_dimensions = m_dimensions;

	int voxelCount = m_dimensions.x * m_dimensions.y * m_dimensions.z;
	size_t byteSize = sizeof(Rgba) * voxelCount;
	newTexture->m_colorData = (Rgba*)malloc(byteSize);

	memcpy(newTexture->m_colorData, m_colorData, byteSize);

	// Collision
	if (m_collisionFlags != nullptr)
	{
		newTexture->m_collisionFlags = (uint32_t*)malloc(sizeof(uint32_t) * m_dimensions.y * m_dimensions.z);
		memcpy(newTexture->m_collisionFlags, m_collisionFlags, sizeof(uint32_t) * m_dimensions.y * m_dimensions.z);
	}

	return newTexture;
}

void VoxelSprite::SetColorAtRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation, const Rgba& color)
{
	IntVector3 localCoords = GetLocalCoordsFromRelativeCoords(relativeCoords, relativeOrientation);

	if (!AreLocalCoordsValid(localCoords.x, localCoords.y, localCoords.z))
	{
		ConsoleErrorf("Invalid coords set for sprite");
		return;
	}

	int index = localCoords.y * (m_dimensions.x * m_dimensions.z) + localCoords.z * m_dimensions.x + localCoords.x;
	SetColorAtIndex(index, color);
}

void VoxelSprite::SetColorAtIndex(unsigned int index, const Rgba& color)
{
	m_colorData[index] = color;
	
	int yCoord = index / (m_dimensions.x * m_dimensions.z);
	int leftOver = index % (m_dimensions.x * m_dimensions.z);

	int zCoord = leftOver / m_dimensions.x;
	int xCoord = leftOver % m_dimensions.x;

	if (m_collisionFlags != nullptr)
	{
		uint32_t& flags = m_collisionFlags[yCoord * m_dimensions.z + zCoord];
		int mask = TEXTURE_LEFTMOST_COLLISION_BIT >> xCoord;

		if (color.a == 0)
		{
			mask = ~mask;
			flags &= mask;
		}
		else
		{
			flags |= (TEXTURE_LEFTMOST_COLLISION_BIT >> xCoord);
		}
	}
}

Rgba VoxelSprite::GetColorAtRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation) const
{
	IntVector3 localCoords = GetLocalCoordsFromRelativeCoords(relativeCoords, relativeOrientation);

	if (!AreLocalCoordsValid(localCoords.x, localCoords.y, localCoords.z))
	{
		return Rgba(0, 0, 0, 0);
	}

	int index = localCoords.y * (m_dimensions.x * m_dimensions.z) + localCoords.z * m_dimensions.x + localCoords.x;
	return m_colorData[index];
}

Rgba VoxelSprite::GetColorAtIndex(unsigned int index) const
{
	return m_colorData[index];
}

IntVector3 VoxelSprite::GetBaseDimensions() const
{
	return m_dimensions;
}

IntVector3 VoxelSprite::GetOrientedDimensions(float orientation) const
{
	float cardinalAngle = GetNearestCardinalAngle(orientation);

	IntVector3 orientedDimensions = m_dimensions;

	if (cardinalAngle == 90.f || cardinalAngle == 270.f)
	{
		orientedDimensions.x = m_dimensions.z;
		orientedDimensions.z = m_dimensions.x;
	}

	return orientedDimensions;
}

unsigned int VoxelSprite::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}

uint32_t VoxelSprite::GetCollisionByteForRow(int referenceY, int referenceZ, float referenceOrientation) const
{
	float cardinalAngle = GetNearestCardinalAngle(referenceOrientation);

	IntVector3 localCoords = GetLocalCoordsFromRelativeCoords(IntVector3(0, referenceY, referenceZ), referenceOrientation);

	ASSERT_OR_DIE(AreLocalCoordsValid(localCoords.x, localCoords.y, localCoords.z), "Voxel texture received bad coords");

	if (m_collisionFlags == nullptr)
	{
		return 0;
	}
	
	uint32_t flags = 0;

	if (cardinalAngle == 90.f) 
	{
		// Iterate across the 32-bit fields to grab a bit out of each, for the entire Z dimension
		// Start low
		IntVector3 orientedDimensions = GetOrientedDimensions(referenceOrientation);
		uint32_t mask = TEXTURE_LEFTMOST_COLLISION_BIT >> referenceZ;
		for (int zIndex = orientedDimensions.z; zIndex >= 0; --zIndex)
		{
			uint32_t localRowFlags = m_collisionFlags[referenceY * m_dimensions.z + zIndex];
			flags |= (((localRowFlags & mask) << referenceZ) >> zIndex);
		}
	}
	else if (cardinalAngle == 180.f) // Flip the Z
	{
		int invertedZ = m_dimensions.z - referenceZ - 1;
		uint32_t backwardsFlags = m_collisionFlags[referenceY * m_dimensions.z + invertedZ];

		flags = GetBitsReversed(backwardsFlags);
		int diff = 32 - m_dimensions.x;
		flags = flags << diff;
	}
	else if (cardinalAngle == 270.f)
	{
		// Iterate across the 32-bit fields to grab a bit out of each, for the entire Z dimension
		// Start low
		IntVector3 orientedDimensions = GetOrientedDimensions(referenceOrientation);
		uint32_t mask = TEXTURE_LEFTMOST_COLLISION_BIT >> (orientedDimensions.x - referenceZ - 1);
		for (int zIndex = 0; zIndex < orientedDimensions.z; ++zIndex)
		{
			uint32_t localRowFlags = m_collisionFlags[referenceY * m_dimensions.z + zIndex];
			flags |= (((localRowFlags & mask) << referenceZ) >> zIndex);
		}
	}
	else // 0.f Degree case - Just return them as they are
	{
		flags = m_collisionFlags[referenceY * m_dimensions.z + referenceZ];
	}

	return flags;
}

bool VoxelSprite::DoLocalCoordsHaveCollision(const IntVector3& coords) const
{
	if (m_collisionFlags == nullptr)
	{
		return false;
	}

	int index = coords.y * m_dimensions.z + coords.z;
	int bitOffset = MAX_TEXTURE_VOXEL_WIDTH - coords.x - 1;
	uint32_t flags = m_collisionFlags[index];

	bool hasCollision = ((flags & (1 << bitOffset)) != 0);
	return hasCollision;
}


//-----------------------------------------------------------------------------------------------
// Returns the sprite given by spriteName, nullptr if it doesn't exist
//
const VoxelSprite* VoxelSprite::GetVoxelSprite(const std::string& spriteName)
{
	bool spriteExists = s_sprites.find(spriteName) != s_sprites.end();

	if (spriteExists)
	{
		return s_sprites.at(spriteName);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns a clone of the sprite given by spriteName, nullptr if it doesn't exist
//
VoxelSprite* VoxelSprite::CreateVoxelSpriteClone(const std::string& spriteName)
{
	const VoxelSprite* spriteToClone = GetVoxelSprite(spriteName);

	if (spriteToClone != nullptr)
	{
		return spriteToClone->Clone();
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Loads the voxel sprites specified in the given xml file
//
void VoxelSprite::LoadSpriteFile(const std::string& filename)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());
	ASSERT_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: VoxelSprite::LoadVoxelSpriteFromFile() couldn't load file %s", filename.c_str()));

	// Get the texture file and the name out of the xml document
	const XMLElement* rootElement = document.RootElement();
	ASSERT_OR_DIE(rootElement != nullptr, Stringf("Error: VoxelSprite::LoadVoxelSpriteFromFile() loaded file with no root element: %s", filename.c_str()));

	const XMLElement* spriteElement = rootElement->FirstChildElement();
	while (spriteElement != nullptr)
	{
		// Get the sprite name
		std::string spriteName = ParseXmlAttribute(*spriteElement, "name");
		ASSERT_OR_DIE(spriteName.size() > 0, Stringf("Error: VoxelSprite::LoadVoxelSpritesFromFile() found sprite with no name in %s", filename.c_str()));

		// Get the sprite texture file
		std::string modelFileName = ParseXmlAttribute(*spriteElement, "file");
		ASSERT_OR_DIE(modelFileName.size() > 0, Stringf("Error: VoxelSprite::LoadVoxelSpritesFromFile() found sprite with no file in %s", filename.c_str()));

		// Construct the sprite
		VoxelSprite* sprite = new VoxelSprite();
		sprite->m_name = spriteName;
		sprite->CreateFromFile(modelFileName.c_str(), true);

		// Add the sprite to the registry
		s_sprites[spriteName] = sprite;

		// Move to the next element
		spriteElement = spriteElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the local coords of the voxel referenced by the relative coordinates in the relative
// orientation space
//
IntVector3 VoxelSprite::GetLocalCoordsFromRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation) const
{
	float cardinalAngle = GetNearestCardinalAngle(relativeOrientation);

	IntVector3 localCoords = relativeCoords;

	if (cardinalAngle == 90.f)
	{
		localCoords.x = relativeCoords.z;
		localCoords.z = m_dimensions.z - relativeCoords.x - 1;
	}
	else if (cardinalAngle == 180.f)
	{
		localCoords.x = m_dimensions.x - relativeCoords.x - 1;
		localCoords.z = m_dimensions.z - relativeCoords.z - 1;
	}
	else if (cardinalAngle == 270.f)
	{
		localCoords.x = m_dimensions.x - relativeCoords.z - 1;
		localCoords.z = relativeCoords.x;
	}

	// 0.f degree case means relative coords are the local coords, so just return

	return localCoords;
}



bool VoxelSprite::AreLocalCoordsValid(int x, int y, int z) const
{
	if (x < 0 || x >= m_dimensions.x)
	{
		return false;
	}

	if (y < 0 || y >= m_dimensions.y)
	{
		return false;
	}

	if (z < 0 || z >= m_dimensions.z)
	{
		return false;
	}

	return true;
}
