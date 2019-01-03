#include <string>

#define NUM_SCORES_PER_LEADERBOARD (5)

class Leaderboard
{
public:
	//-----Private Data-----

	std::string m_name;
	int m_scores[NUM_SCORES_PER_LEADERBOARD];
};