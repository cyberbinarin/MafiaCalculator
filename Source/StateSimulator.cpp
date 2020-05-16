#include "StateSimulator.h"

#include <array>
#include <cassert>
#include <map>
#include "GameState.h"
#include "StateMerger.h"

namespace {

	enum class Affiliation {
		None,
		Village,
		Mafia
	};

	template<typename T>
	void _RemoveAtSwap(std::vector<T>& a_vect, unsigned char a_index)
	{
		std::iter_swap(a_vect.begin() + a_index, a_vect.end() - 1);
		a_vect.pop_back();
	}

	bool _ShouldDetectiveExposeThemselves(const PersonState& a_detectiveState, const GameState& a_currentState)
	{
		if (a_detectiveState.m_exposedAmount == PersonExposedAmount::RoleExposed)
			return false;
		
		//If any other detective are more exposed, return false
		for (const auto& otherPerson : a_currentState.m_personStates)
		{
			if (otherPerson->m_role == Role::Detective && otherPerson->m_id != a_detectiveState.m_id && otherPerson->m_exposedAmount > a_detectiveState.m_exposedAmount)
				return false;
		}
		for (const auto& otherPerson : a_currentState.m_personStates)
		{
			//If a persons exposed amount is exactly investigated, they have been investigated but not exposed
			if (otherPerson->m_role == Role::Mafia && otherPerson->m_exposedAmount == PersonExposedAmount::Investigated)
				return true;
		}
		return false;
	}

	Affiliation _GetWinner(const GameState& a_state) {
		unsigned char mafiaCount{0};
		for (const auto& person : a_state.m_personStates)
		{
			if (person->m_role == Role::Mafia)
				++mafiaCount;
		}
		if (mafiaCount == 0)
			return Affiliation::Village;

		if (mafiaCount * 2 >= static_cast<unsigned char>(a_state.m_personStates.size()))
			return Affiliation::Mafia;

		return Affiliation::None;
	}

	void _DivideEachProbabilityWithSize(std::vector<GameState>& a_states)
	{
		for(auto& state : a_states) {
			state.m_inProbability /= a_states.size();
		}
	}

	void _AdvanceAllPhases(std::vector<GameState>& a_states)
	{
		for(auto& state : a_states) {
			state.m_dayPhase = static_cast<DayPhase>((static_cast<unsigned char>(state.m_dayPhase) + 1) % static_cast<unsigned char>(DayPhase::Count));
		}
	}

	std::vector<GameState> _GetFollowingStatesDay(GameState a_state)
	{
		std::vector<GameState> result;

		//Detectives can expose themselves. And then, any exposed detectives exposes all people they have investigated
		for (auto& personState : a_state.m_personStates)
		{
			if (personState->m_role == Role::Detective) {
				if (_ShouldDetectiveExposeThemselves(*personState, a_state))
				{
					personState = std::make_shared<const PersonState>(*personState, PersonExposedAmount::RoleExposed);
				}
				if (personState->m_exposedAmount == PersonExposedAmount::RoleExposed)
				{
					for (auto& otherPersons : a_state.m_personStates)
					{
						if (otherPersons->m_exposedAmount == PersonExposedAmount::Investigated)
						{
							otherPersons = std::make_shared<const PersonState>(*otherPersons, PersonExposedAmount::AffiliationExposed);
						}
					}
					break;
				}
			}
		}

		//If there are any exposed Mafia, hang one
		for (unsigned char i{0}; i < a_state.m_personStates.size(); ++i)
		{
			if (a_state.m_personStates[i]->m_exposedAmount >= PersonExposedAmount::AffiliationExposed && a_state.m_personStates[i]->m_role == Role::Mafia) {
				auto newGameState{a_state};
				_RemoveAtSwap(newGameState.m_personStates, i);
				result.push_back(newGameState);
				break;
			}
		}

		if (result.size() == 0)
		{
			//Hanging random people, except for exposed people
			for (unsigned char i{0}; i < a_state.m_personStates.size(); ++i) {
				if (a_state.m_personStates[i]->m_exposedAmount < PersonExposedAmount::AffiliationExposed)
				{
					auto newGameState{a_state};
					_RemoveAtSwap(newGameState.m_personStates, i);
					result.push_back(newGameState);
				}
			}
		}

		_DivideEachProbabilityWithSize(result);
		_AdvanceAllPhases(result);
		StateMerger::MergeSimilarStates(result);

		return result;
	}

	std::vector<GameState> _GetFollowingStatesNightDetective(const GameState& a_inState)
	{
		bool anyDetectives{false};
		for (const auto& personState : a_inState.m_personStates)
		{
			if (personState->m_role == Role::Detective)
			{
				anyDetectives = true;
				break;
			}
		}

		std::vector<GameState> states;

		if (anyDetectives) {
			for (unsigned char i{0}; i < a_inState.m_personStates.size(); ++i)
			{
				const auto& personState{a_inState.m_personStates[i]};
				if (personState->m_role != Role::Detective && personState->m_exposedAmount == PersonExposedAmount::None)
				{
					auto newGameState{a_inState};
					newGameState.m_personStates[i] = std::make_shared<PersonState>(*personState, PersonExposedAmount::Investigated);
					states.push_back(std::move(newGameState));
				}
			}
		}

		if (states.empty())
			states.push_back(a_inState);

		_DivideEachProbabilityWithSize(states);
		_AdvanceAllPhases(states);
		StateMerger::MergeSimilarStates(states);

		return states;
	}

	std::vector<GameState> _GetFollowingStatesNightMafia(const GameState& a_inState)
	{
		std::vector<GameState> result;

		//If there are any exposed detectives, murder one
		for (unsigned char i{0}; i < a_inState.m_personStates.size(); ++i)
		{
			if (a_inState.m_personStates[i]->m_exposedAmount == PersonExposedAmount::RoleExposed && a_inState.m_personStates[i] -> m_role == Role::Detective)
			{
				auto newGameState{a_inState};
				_RemoveAtSwap(newGameState.m_personStates, i);
				result.push_back(newGameState);
				break;
			}
		}

		if (result.size() == 0)
		{
			//If there are any exposed villagers, murder one
			for (unsigned char i{0}; i < a_inState.m_personStates.size(); ++i)
			{
				if (a_inState.m_personStates[i]->m_exposedAmount >= PersonExposedAmount::AffiliationExposed && a_inState.m_personStates[i] -> m_role == Role::Villager)
				{
					auto newGameState{a_inState};
					_RemoveAtSwap(newGameState.m_personStates, i);
					result.push_back(newGameState);
					break;
				}
			}

			if (result.size() == 0)
			{
				//Mudering random people, except for mafia
				for (unsigned char i{0}; i < a_inState.m_personStates.size(); ++i) {
					if (a_inState.m_personStates[i] -> m_role != Role::Mafia)
					{
						auto newGameState{a_inState};
						_RemoveAtSwap(newGameState.m_personStates, i);
						result.push_back(newGameState);
					}
				}
			}
		}

		_DivideEachProbabilityWithSize(result);
		_AdvanceAllPhases(result);
		StateMerger::MergeSimilarStates(result);

		return result;
	}

}

namespace StateSimulator
{

	std::vector<GameState> GetFollowingStates(const GameState& a_inState)
	{
		switch(a_inState.m_dayPhase)
		{
			case DayPhase::Day:
				return _GetFollowingStatesDay(a_inState);
			case DayPhase::NightDetective:
				return _GetFollowingStatesNightDetective(a_inState);
			case DayPhase::NightMafia:
				return _GetFollowingStatesNightMafia(a_inState);
		}
		assert(false);
		return {a_inState};
	}

	void GetWinProbs(const GameState& a_state, long double& a_villageWinProb, long double& a_mafiaWinProb)
	{
		a_villageWinProb = 0;
		a_mafiaWinProb = 0;
		std::vector<GameState> states{a_state};
		while (!states.empty())
		{
			const auto winner{_GetWinner(states.back())};
			if (winner == Affiliation::Mafia)
			{
				a_mafiaWinProb += states.back().m_inProbability;
				states.pop_back();
			}
			else if (winner == Affiliation::Village)
			{
				a_villageWinProb += states.back().m_inProbability;
				states.pop_back();
			}
			else
			{
				const auto newStates{GetFollowingStates(states.back())};
				states.pop_back();
				states.insert(states.end(), newStates.begin(), newStates.end());
			}
		}
	}

}
