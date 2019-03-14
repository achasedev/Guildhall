/************************************************************************/
/* File: Leaderboard.hpp
/* Author: Andrew Chase
/* Date: March 14th, 2019
/* Description: Class for representing a set of scores, by player count
/************************************************************************/
#include <string>

#define NUM_SCORES_PER_LEADERBOARD (5)

class Leaderboard
{
public:
	//-----Public Data-----

	std::string m_name;
	int m_scores[NUM_SCORES_PER_LEADERBOARD];
};