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

class AnimationSet
{
public:
	//-----Public Methods-----
	
	void						LoadFromFile(const char* filename);

	std::string					GetName() const;
	bool						TranslateAlias(const std::string& alias, std::string& out_translation) const;

	static void					LoadAnimationSet(const std::string& filename);
	static const AnimationSet*	GetAnimationSet(const std::string& setName);


private:
	//-----Private Methods-----

	// Construction only allowed in LoadAnimationSet()
	AnimationSet(const XMLElement& setElement);

	// Only used in LoadAnimationSet()
	static void AddAnimation(const AnimationSet* animationSet);


private:
	//-----Private Data-----
	
	std::string m_name;
	std::map<std::string, std::vector<std::string>> m_translations;

	static std::map<std::string, const AnimationSet*> s_animationSets;

};
