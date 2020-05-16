#pragma once

#include <memory>
#include <vector>

struct GameState;

namespace StateSimulator
{
	std::vector<std::unique_ptr<const GameState>> GetFollowingStates(std::unique_ptr<const GameState> a_inState);
	void GetWinProbs(const GameState& a_state, long double& a_villageWinProb, long double& a_mafiaWinProb);
};

