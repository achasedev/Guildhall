/************************************************************************/
/* File: AnimationSet.cpp
/* Author: Andrew Chase
/* Date: March 14th, 2017
/* Description: Implementation of the AnimationSet class
/************************************************************************/
#include "Game/GameCommon.hpp"
#include "Game/AnimationSet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/XmlUtilities.hpp"

// ALL AnimationSets are stored here
std::map<std::string, AnimationSet*> AnimationSet::s_animationSets;


//-----------------------------------------------------------------------------------------------
// Returns the name of this AnimationSet
//
std::string AnimationSet::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns a name corresponding to the given alias, randomly choosing one if multiple exist
// Returns an empty string if the alias doesn't exist
//
std::string AnimationSet::TranslateAlias(const std::string& alias) const
{
	// See if the alias exists, if not return an empty string
	if (m_translations.find(alias) == m_translations.end())
	{
		return std::string();
	}

	// Alias exists, so find a random animation name
	const std::vector<std::string>& names = m_translations.at(alias);

	int index = GetRandomIntLessThan((int) names.size());
	return names[index];
}


//-----------------------------------------------------------------------------------------------
// Loads an AnimationSet XML file and populates the AnimationSet registry
//
void AnimationSet::LoadAnimationSet(const std::string& filePath)
{
	// Load the file
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: AnimationSet::LoadAnimationSet() couldn't load file \"%s\"", filePath.c_str()));

	// Get the Set name
	XMLElement* setElement	= document.RootElement();
	std::string setName		= ParseXmlAttribute(*setElement, "name");

	// Create and add the AnimationSet to the registry
	AnimationSet* animationSet = new AnimationSet(setName, *setElement);
	AddAnimationSet(animationSet);
}


//-----------------------------------------------------------------------------------------------
// Returns the Set given by setName if it exists, nullptr otherwise
//
AnimationSet* AnimationSet::GetAnimationSet(const std::string& setName)
{
	bool setExists = (s_animationSets.find(setName) != s_animationSets.end());

	if (setExists)
	{
		return s_animationSets.at(setName);
	}
	
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Construct from XML - populates the local map of alias-name pairs
//
AnimationSet::AnimationSet(const std::string name, const XMLElement& setElement)
	: m_name(name)
{
	const XMLElement* aliasElement = setElement.FirstChildElement();

	while (aliasElement != nullptr)
	{
		std::string alias = ParseXmlAttribute(*aliasElement, "alias");

		const XMLElement* animationElement = aliasElement->FirstChildElement();
		while (animationElement != nullptr)
		{
			std::string animationName = ParseXmlAttribute(*animationElement, "name");
			m_translations[alias].push_back(animationName);

			animationElement = animationElement->NextSiblingElement();
		}

		aliasElement = aliasElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Adds the given AnimationSet to the registry, checking for duplicates
//
void AnimationSet::AddAnimationSet(AnimationSet* animationSet)
{
	std::string setName = animationSet->GetName();
	bool alreadyExists = (s_animationSets.find(setName) != s_animationSets.end());
	GUARANTEE_RECOVERABLE(!alreadyExists, Stringf("Warning: AnimationSet::AddDefinition() attempted to add duplicate AnimationSet \"%s\"", setName.c_str()));

	s_animationSets[setName] = animationSet;
}
