#include "Game/Framework/GameCommon.hpp"
#include "Game/Environment/BlockDefinition.hpp"

std::map<std::string, const BlockDefinition*> BlockDefinition::s_definitions;


const BlockDefinition* BlockDefinition::GetDefinitionByName(const std::string& name)
{
	bool definitionExists = s_definitions.find(name) != s_definitions.end();

	GUARANTEE_OR_DIE(definitionExists, Stringf("Error: BlockDefinition::GetDefinitionByName couldn't find definition of name \"%s\"", name.c_str()));

	return s_definitions[name];
}
