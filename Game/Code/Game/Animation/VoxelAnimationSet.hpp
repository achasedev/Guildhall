/************************************************************************/
/* File: AnimationSet.hpp
/* Author: Andrew Chase
/* Date: October 1st, 2018
/* Description: Class to represent a collection of aliases for animations
/************************************************************************/
#pragma once
#include <map>
#include <vector>
#include <string>
#include "Engine/Core/Utility/XmlUtilities.hpp"

class VoxelAnimationSet
{
public:
	//-----Public Methods-----

	// Accessors
	std::string						GetName() const;
	bool							TranslateAlias(const std::string& alias, std::string& out_translation) const;

	// Statics
	static const VoxelAnimationSet*	LoadSet(const std::string& filename);
	static const VoxelAnimationSet*	GetAnimationSet(const std::string& setName);


private:
	//-----Private Methods-----

	// Construction only allowed in LoadAnimationSet()
	VoxelAnimationSet(const XMLElement& setElement);


private:
	//-----Private Data-----
	
	std::string m_name;
	std::map<std::string, std::vector<std::string>> m_translations;

	static std::map<std::string, const VoxelAnimationSet*> s_animationSets;

};
