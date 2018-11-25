/************************************************************************/
/* File: VoxelTerrain.hpp
/* Author: Andrew Chase
/* Date: November 25th 2018
/* Description: Class to represent the terrain of a world
/************************************************************************/
#pragma once
#include <map>
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
class VoxelTexture;

class VoxelTerrain
{
public:
	//-----Public Methods-----
	

	static VoxelTerrain* GetTerrainClone(const std::string& terrainName);
	static void LoadTerrain(const XMLElement& terrainElement);

	// Mutators
	void AddVoxel(const IntVector3& coords, const Rgba& color);
	Rgba RemoveVoxel(const IntVector3& coords);

	// Accessors
	int GetHeightAtCoords(const IntVector2& coords);
	Rgba GetColorAtCoords(const IntVector3& coords);


public:
	//-----Public Data-----
	

private:
	//-----Private Methods-----
	
	VoxelTerrain() {};
	VoxelTerrain* Clone() const;

	
private:
	//-----Private Data-----
	
	std::string m_name;
	VoxelTexture* m_texture = nullptr;

	static const IntVector3 TERRAIN_DIMENSIONS;
	static std::map<std::string, VoxelTerrain*> s_terrains;

};
