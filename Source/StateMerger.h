#pragma once

#include <vector>
#include <memory>

struct GameState;

namespace StateMerger
{

	void MergeSimilarStates(std::vector<GameState>& a_states);

}
