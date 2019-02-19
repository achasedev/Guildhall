#pragma once
#include "Engine/Math/IntVector3.hpp"
#include <map>
#include <string>
#include <stdint.h>

#define MAX_TEXTURE_VOXEL_WIDTH (32)
#define MAX_TEXTURE_BYTE_WIDTH (MAX_TEXTURE_VOXEL_WIDTH / 8)
#define TEXTURE_LEFTMOST_COLLISION_BIT (0x80000000)

class Rgba;

class VoxelSprite
{
	friend class VoxelTerrain;

public:
	//-----Public Methods-----

	VoxelSprite();
	~VoxelSprite();

	bool					CreateFromFile(const char* filename, bool createCollisionMatrix);
	bool					CreateFromColorStream(const Rgba* colors, const IntVector3& dimensions, bool createCollisionMatrix);
	VoxelSprite*			Clone() const;

	// Mutators
	void			SetColorAtRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation, const Rgba& color);

	Rgba			GetColorAtIndex(unsigned int index) const;					// Does not account of orientation!!
	void			SetColorAtIndex(unsigned int index, const Rgba& color);		// Does not account of orientation!!

	// Accessors
	Rgba			GetColorAtRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation) const;
	IntVector3		GetBaseDimensions() const;
	IntVector3		GetOrientedDimensions(float orientation) const;
	unsigned int	GetVoxelCount() const;
	uint32_t		GetCollisionByteForRow(int referenceY, int referenceZ, float referenceOrientation) const;

	// Producers
	bool			DoLocalCoordsHaveCollision(const IntVector3& coords) const;


	// Statics
	static const VoxelSprite*	GetVoxelSprite(const std::string& spriteName);
	static VoxelSprite*			CreateVoxelSpriteClone(const std::string& spriteName);

	static void					LoadSpriteFile(const std::string& filename);


protected:
	//-----Protected Methods-----

	IntVector3	GetLocalCoordsFromRelativeCoords(const IntVector3& relativeCoords, float relativeOrientation) const;
	bool		AreLocalCoordsValid(int x, int y, int z) const;

	static void AddSpriteToRegistry(VoxelSprite* sprite);


protected:
	//-----Protected Data-----

	std::string m_name;
	uint32_t*	m_collisionBitRows = nullptr;
	Rgba*		m_voxelColors = nullptr;
	IntVector3	m_dimensions;

	static std::map<std::string, const VoxelSprite*> s_sprites;

};
