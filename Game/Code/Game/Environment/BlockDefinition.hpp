#pragma once
#include <map>
#include <string>

class Material;

class BlockDefinition
{
	friend class Block;	// For ease of access

public:
	//-----Public Methods-----

	static void LoadDefinitions(const char* filename);

	static const BlockDefinition* GetDefinitionByName(const std::string& name);


private:
	//-----Private Data-----

	Material* m_definitionMaterial = nullptr;	// The visual of this block - since all blocks will be cubes, mesh doesn't change (for now)

	static std::map<std::string, const BlockDefinition*> s_definitions;

};
