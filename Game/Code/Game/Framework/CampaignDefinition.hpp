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
#include "Game/Framework/CampaignStageData.hpp"

class CampaignDefinition
{
public:
	//-----Public Methods-----


private:
	//-----Private Methods-----

	CampaignDefinition() {};
	CampaignDefinition(const std::string& filePath);


public:
	//-----Public Data-----

	std::string						m_name;
	std::string						m_backgroundMusicTrack;
	std::vector<CampaignStageData>	m_stages;

	static std::map<std::string, const CampaignDefinition*>  s_campaignDefinitions;

};
