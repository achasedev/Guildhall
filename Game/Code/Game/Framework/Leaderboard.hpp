/************************************************************************/
/* File: Leaderboard.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2019
/* Description: Class for representing a set of scores, by player count
/************************************************************************/
#include <string>
#include "Game/Framework/World.hpp"

#define NUM_SCORES_PER_SCOREBOARD (5)

class ScoreBoard
{
public:
	//-----Public Data-----

	ScoreBoard()
	{
		for (int i = 0; i < NUM_SCORES_PER_SCOREBOARD; ++i)
		{
			m_scores[i] = 0;
		}
	}

	std::string m_name;
	int m_scores[NUM_SCORES_PER_SCOREBOARD];

};

class Leaderboard
{
public:
	//-----Public Data-----

	std::string m_name;
	ScoreBoard	m_scoreboards[MAX_PLAYERS];

};
