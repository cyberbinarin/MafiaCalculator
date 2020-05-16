#pragma once

#include <vector>
#include "DetectiveState.h"

enum class DayPhase : unsigned char {
	Day,
	NightDetective,
	NightMafia,

	Count
};

struct GameState
{
	//The probability that this state would be reached
	long double m_inProbability{1};

	std::vector<std::shared_ptr<const PersonState>> m_personStates;

	DayPhase m_dayPhase{DayPhase::Day};

	GameState() = default;
	GameState(const GameState&) = default;
	GameState(GameState&&) = default;

	GameState& operator=(const GameState&) = default;
	GameState& operator=(GameState&&) = default;
};
