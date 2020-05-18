#include "StateMerger.h"

#include <map>
#include "GameState.h"

namespace
{

	struct PersonSimilarityState
	{
		Role m_role;
		PersonExposedAmount m_exposedAmount;

		PersonSimilarityState(Role a_role, PersonExposedAmount m_exposedAmount)
			: m_role(a_role)
			, m_exposedAmount(m_exposedAmount)
		{
		}

		bool operator==(const PersonSimilarityState& a_rhs) const
		{
			return m_role == a_rhs.m_role && m_exposedAmount == a_rhs.m_exposedAmount;
		}
	};

	struct SimilarityStateComparer {
		bool operator()(const PersonSimilarityState& a_lhs, const PersonSimilarityState& a_rhs) const
		{
			return a_lhs.m_role < a_rhs.m_role || (a_lhs.m_role == a_rhs.m_role && a_lhs.m_exposedAmount < a_rhs.m_exposedAmount);
		}
	};

	PersonSimilarityState _CreateSimilarityState(const PersonState& a_state)
	{
		return {a_state.GetRole(), a_state.m_exposedAmount};
	}

	std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> _CountSimilarityStates(const GameState& a_state)
	{
		std::map<PersonSimilarityState, unsigned char, SimilarityStateComparer> result{};
		for (const auto& personState : a_state.m_personStates)
		{
			++result[_CreateSimilarityState(personState)];
		}
		return result;
	}

	bool _StatesAreSimilar(const GameState& a_lhs, const GameState& a_rhs)
	{
		return _CountSimilarityStates(a_lhs) == _CountSimilarityStates(a_rhs);
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