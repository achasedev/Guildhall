/************************************************************************/
/* File: VoxelTerrain.hpp
/* Author: Andrew Chase
/* Date: January 29th, 2018
/* Description: Static class to represent the grounds used for maps
/************************************************************************/
#include <map>
#include <vector>
#include <string>

class VoxelSprite;

class VoxelTerrain : public VoxelSprite
{
public:
	//-----Public Methods-----

	static void				LoadTerrainFile(const std::string& terrainFile);
	static VoxelTerrain*	CreateVoxelTerrainCloneForName(const std::string& name);
	static VoxelTerrain*	CreateVoxelTerrainCloneForType(const std::string& type);


private:
	//-----Private Methods-----

	// Helper for assigning a type to a terrain when creating one from a sprite
	static std::string FindTypeForTerrain(VoxelTerrain* terrain);


private:
	//-----Private Data-----

	std::string m_terrainType; // The "category" of the terrain

	// Groups terrain names by type, so type -> vector of names under that type
	static std::map<std::string, std::vector<std::string>> s_terrainsByType;

};
