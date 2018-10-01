#include "Game/Animation/AnimationSet.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

AnimationSet::AnimationSet(const XMLElement& setElement)
{
	std::string setName = ParseXmlAttribute(setElement, "name");

	if (setName.size() == 0)
	{
		ERROR_AND_DIE("AnimationSet::AnimationSet() read file %s with no name specified in root element", filename.c_str());
	}

	m_name = setName;

	// Read in each alias element and parse the data
	const XMLElement* aliasElement = setElement.FirstChildElement();

	while (aliasElement != nullptr)
	{
		std::string alias = ParseXmlAttribute(*aliasElement, "alias");

		if (alias.size() == 0)
		{
			ERROR_AND_DIE("AnimationSet::AnimationSet() found alias element with no alias specified - set was %s", setName.c_str());
		}

		const XMLElement* animationElement = aliasElement->FirstChildElement();

		while (animationElement != nullptr)
		{
			std::string animationName = ParseXmlAttribute(*animationElement, "name");

			if (animationName.size() == 0)
			{
				ERROR_AND_DIE("AnimationSet::AnimationSet() found animation element with no name specified - set was %s", setName.c_str());
			}

			m_translations[alias].push_back(animationName);

			animationElement = animationElement->NextSiblingElement();
		}

		aliasElement = aliasElement->NextSiblingElement();
	}
}


std::string AnimationSet::GetName() const
{
	return m_name;
}

bool AnimationSet::TranslateAlias(const std::string& alias, std::string& out_translation) const
{
	bool aliasExists = m_translations.find(alias) != m_translations.end();

	if (aliasExists)
	{
		const std::vector<std::string>& possibleNames = m_translations.at(alias);
		int randomIndex = GetRandomIntLessThan((int)possibleNames.size());

		out_translation = possibleNames[randomIndex];
	}

	return aliasExists;
}

const AnimationSet* AnimationSet::LoadSetFromFile(const std::string& filename)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filename.c_str());

	if (error != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE("AnimationSet::LoadSetFromFile() couldn't open file %s", filename.c_str());
		return nullptr;
	}

	const XMLElement* rootElement = document.RootElement();

	AnimationSet* newSet = new AnimationSet(*rootElement);
	s_animationSets[newSet->GetName()] = newSet;

	return newSet;
}

const AnimationSet* AnimationSet::GetAnimationSet(const std::string& setName)
{
	bool setExists = s_animationSets.find(setName) != s_animationSets.end();

	if (setExists)
	{
		return s_animationSets.at(setName);
	}

	return nullptr;
}

