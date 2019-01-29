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

	static VoxelTerrain LoadTerrainFile(const std::string& terrainFile);

	static VoxelTerrain* CreateVoxelTerrainCloneForName(const std::string& name);

	// Get the const sprite

	// Since we can read it, copy & paste the clone function to do the same with a newed off terrain

	// Set the type member of the terrain

	// return it


	static VoxelTerrain* CreateVoxelTerrainCloneForType(const std::string& type);


private:
	//-----Private Methods-----



private:
	//-----Private Data-----

	std::string m_terrainType;

	static std::map<std::string, std::vector<std::string>> m_terrainsByType;

};
