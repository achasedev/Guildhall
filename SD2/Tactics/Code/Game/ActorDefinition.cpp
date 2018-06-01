#include "Game/IsoSprite.hpp"
#include "Game/PlayAction.hpp"
#include "Game/GameCommon.hpp"
#include "Game/AnimationSet.hpp"
#include "Game/ActorDefinition.hpp"


// Static map for all actor definitions
std::map<std::string, ActorDefinition*> ActorDefinition::s_definitions;


ActorDefinition::ActorDefinition(const XMLElement& defElement)
{
	// Parse name
	m_name = ParseXmlAttribute(defElement, "name");

	// Parse attributes
	const XMLElement* attribElement = defElement.FirstChildElement("attributes");
	if (attribElement != nullptr)
	{
		m_maxHealth		= ParseXmlAttribute(*attribElement, "maxhealth", 100);
		m_moveSpeed		= ParseXmlAttribute(*attribElement, "movespeed", 5);
		m_jumpHeight	= ParseXmlAttribute(*attribElement, "jump", 2);
		m_height		= ParseXmlAttribute(*attribElement, "height", 2);
		m_strength		= ParseXmlAttribute(*attribElement, "strength", 10);
		m_actionSpeed	= ParseXmlAttribute(*attribElement, "actionspeed", 30.f);
		m_blockRate		= ParseXmlAttribute(*attribElement, "blockrate", 0.f);
		m_critChance	= ParseXmlAttribute(*attribElement, "critchance", 0.f);
	}

	// Parse Animation Data
	const XMLElement* animElement = defElement.FirstChildElement("animation");
	if (animElement != nullptr)
	{
		std::string animSetName = ParseXmlAttribute(*animElement, "animationset");
		m_animationSet = AnimationSet::GetAnimationSet(animSetName);

		std::string defaultSpriteName = ParseXmlAttribute(*animElement, "defaultsprite");
		m_defaultSprite = IsoSprite::GetIsoSprite(defaultSpriteName);
	}

	// Parse Actions
	const XMLElement* actionsElement = defElement.FirstChildElement("actions");
	if (actionsElement != nullptr)
	{
		const XMLElement* subElement = actionsElement->FirstChildElement();
		while (subElement != nullptr)
		{
			std::string actionNameText = ParseXmlAttribute(*subElement, "name");
			eActionName actionName = PlayAction::GetActionNameFromText(actionNameText);

			m_actions.push_back(actionName);
			subElement = subElement->NextSiblingElement();
		}
	}
}

std::string ActorDefinition::GetName() const
{
	return m_name;
}

int ActorDefinition::GetMaxHealth() const
{
	return m_maxHealth;
}

int ActorDefinition::GetMoveSpeed() const
{
	return m_moveSpeed;
}

int ActorDefinition::GetJumpHeight() const
{
	return m_jumpHeight;
}

int ActorDefinition::GetActorHeight() const
{
	return m_height;
}

int ActorDefinition::GetStrength() const
{
	return m_strength;
}

float ActorDefinition::GetActionSpeed() const
{
	return m_actionSpeed;
}

float ActorDefinition::GetBlockRate() const
{
	return m_blockRate;
}

float ActorDefinition::GetCritChance() const
{
	return m_critChance;
}

AnimationSet* ActorDefinition::GetAnimationSet() const
{
	return m_animationSet;
}

IsoSprite* ActorDefinition::GetDefaultSprite() const
{
	return m_defaultSprite;
}

const std::vector<eActionName>& ActorDefinition::GetActions() const
{
	return m_actions;
}

void ActorDefinition::LoadDefinitions(const std::string& filePath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filePath.c_str());
	GUARANTEE_OR_DIE(error == tinyxml2::XML_SUCCESS, Stringf("Error: ActorDefinition::LoadDefinitions() couldn't load file \"%s\"", filePath.c_str()));

	const XMLElement* rootElement = document.RootElement();
	const XMLElement* defElement = rootElement->FirstChildElement();

	while (defElement != nullptr)
	{
		ActorDefinition* definition = new ActorDefinition(*defElement);
		AddDefinition(definition);

		defElement = defElement->NextSiblingElement();
	}
}

void ActorDefinition::AddDefinition(ActorDefinition* definition)
{
	std::string definitionName = definition->GetName();
	bool alreadyExists = (s_definitions.find(definitionName) != s_definitions.end());
	GUARANTEE_RECOVERABLE(!alreadyExists, Stringf("Warning: ActorDefinition::AddDefinition() attempted to add duplicate definition \"%s\"", definitionName.c_str()));

	s_definitions[definitionName] = definition;
}

ActorDefinition* ActorDefinition::GetDefinition(const std::string& name)
{
	bool definitionExists = (s_definitions.find(name) != s_definitions.end());

	if (definitionExists)
	{
		return s_definitions[name];
	}
	else
	{
		return nullptr;
	}
}

//-----XML Format-----
// <actordefinitions>
// 
//		<actordefinition name="archer_f">
//			<attributes maxhealth="100" movespeed="5" jump="2" height="2" strength="10" actionspeed="30" />
//			<animation animationset="archer_f" defaultsprite="archer_f.idle" />
//			<actions>
//				<action name="attack" />
//				<action name="move" />
//				<action name="wait" />
//			</actions>
//		</actordefinition>
// 
// </actordefinitions>