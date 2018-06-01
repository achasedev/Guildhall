/************************************************************************/
/* File: AnimationSet.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Class to represent a set of name/alias translations
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Engine/Core/XmlUtilities.hpp"


class AnimationSet
{
public:
	//-----Public Methods-----

	std::string GetName() const;
	std::string TranslateAlias(const std::string& alias) const;

	// Statics
	static void				LoadAnimationSet(const std::string& filePath);
	static AnimationSet*	GetAnimationSet(const std::string& setName);


private:
	//-----Private Methods-----

	// Construct from XML - only to be used in LoadDefinitions()
	AnimationSet(const std::string name, const XMLElement& setElement);

	// Only add in LoadDefinitions()
	static void AddAnimationSet(AnimationSet* animationSet);


private:
	//-----Private Data-----

	std::string m_name;
	std::map<std::string, std::vector<std::string>> m_translations;

	// ALL AnimationSets in the system are stored here
	static std::map<std::string, AnimationSet*> s_animationSets;

};
