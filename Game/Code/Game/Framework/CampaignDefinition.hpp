/************************************************************************/
/* File: CampaignDefinition.hpp
/* Author: Andrew Chase
/* Date: February 3rd 2019
/* Description: Class to represent campaign data loaded into memory
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Game/Framework/CampaignStage.hpp"

class CampaignDefinition
{
public:
	//-----Public Methods-----

	static void							LoadCampaign(const std::string filePath);
	static const CampaignDefinition*	GetDefinitionByName(const std::string& name);
	static int							GetCampaignCount();


private:
	//-----Private Methods-----

	CampaignDefinition() {}

	static void AddDefinitionToRegistry(const CampaignDefinition* definition);
	

public:
	//-----Public Data-----

	std::string					m_name;
	std::string					m_backgroundMusicTrack;
	std::vector<CampaignStage*>	m_stages;

	bool						m_hasCharacterSelect = false;
	bool						m_hasLeaderboards = true;
	bool						m_displayedOnMainMenu = true;
	static std::map<std::string, const CampaignDefinition*>  s_campaignDefinitions;

};
