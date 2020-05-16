#include "StateMerger.h"

#include <map>
#include "GameState.h"

namespace
{
	struct PersonSimilarityState
	{
		Role role;
		bool exposed;

		PersonSimilarityState(Role a_role, bool a_exposed)
			: role(a_role)
			, exposed(a_exposed)
		{
		}

		bool operator==(const PersonSimilarityState& a_rhs) const
		{
			return role == a_rhs.role && exposed == a_rhs.exposed;
		}
	};

	struct SimilarityStateComparer {
		bool operator()(const PersonSimilarityState& a_lhs, const PersonSimilarityState& a_rhs) const
		{
			if (a_lhs.exposed && !a_rhs.exposed)
				return false;

			if (!a_lhs.exposed && a_rhs.exposed)
				return true;

			return a_lhs.role < a_rhs.role;
		}
	};

	PersonSimilarityState _CreateSimilarityState(const PersonState& a_state)
	{
		return {a_state.m_role, a_state.m_exposed};
	}

	std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> _CountSimilarityStates(const GameState& a_state)
	{
		std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> result{};
		for (const auto& personState : a_state.m_personStates)
		{
			++result[_CreateSimilarityState(*personState)];
		}
		return result;
	}

	std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> _CountInspectedSimilarityStates(const GameState& a_state,
		const std::set<unsigned char>& a_inspectedIds)
	{
		std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> result{};
		for (const auto& personState : a_state.m_personStates)
		{
			if (a_inspectedIds.find(personState->m_id) != a_inspectedIds.end())
				++result[_CreateSimilarityState(*personState)];
		}
		return result;
	}

	std::vector<const DetectiveState*> _GetDetectives(const GameState& a_state)
	{
		std::vector<const DetectiveState*> result{};
		for (const auto& personState : a_state.m_personStates)
		{
			if (personState->m_role == Role::Detective)
			{
				result.push_back(static_cast<const DetectiveState*>(personState.get()));
			}
		}
		return result;
	}

	bool _StatesAreSimilar(const GameState& a_lhs, const GameState& a_rhs)
	{
		if (_CountSimilarityStates(a_lhs) != _CountSimilarityStates(a_rhs))
			return false;

		const auto lhsDetectives{_GetDetectives(a_lhs)};
		const auto rhsDetectives{_GetDetectives(a_rhs)};
	
		if (lhsDetectives.size() != rhsDetectives.size())
			return false;

		if (lhsDetectives.size() == 0)
			return true;

		//TODO merge states with multiple detectives
		if (lhsDetectives.size() > 1)
			return false;

		//Check if the detectives states are the same instance
		if (lhsDetectives.back() == rhsDetectives.back())
			return true;

		return false;

		//return _CountInspectedSimilarityStates(a_lhs, lhsDetectives.back()->m_inspectedPeople)
		//	== _CountInspectedSimilarityStates(a_rhs, rhsDetectives.back()->m_inspectedPeople);
	}

	void _MergeStates(GameState& a_toMergeInto, const GameState& a_toMergeFrom)
	{
		a_toMergeInto.m_inProbability += a_toMergeFrom.m_inProbability;
	}

}

namespace StateMerger
{

	void MergeSimilarStates(std::vector<GameState>& a_states)
	{
		auto current{a_states.begin()};
		auto doneNext{a_states.begin()};
		while(current < a_states.end())
		{
			bool merged{false};
			for (auto doneIt{a_states.begin()}; doneIt < doneNext; ++doneIt)
			{
				if (_StatesAreSimilar(*doneIt, *current))
				{
					_MergeStates(*doneIt, *current);
					merged = true;
					break;
				}
			}
			if (!merged)
			{
				if (doneNext < current)
				{
					(*doneNext) = *current;
				}
				++doneNext;
			}
			++current;
		}
		if (doneNext < current)
			a_states.erase(doneNext, current);
	}

}