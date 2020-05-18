#pragma once

#include <cassert>
#include <memory>
#include "PersonExposedAmount.h"
#include "Roles.h"

class PersonState {
public:
	PersonExposedAmount m_exposedAmount{PersonExposedAmount::None};

	PersonState(unsigned char a_id, Role a_role)
		: m_id(a_id)
		, m_role(a_role)
	{
	}

	PersonState(const PersonState& a_state, PersonExposedAmount a_exposedAmount)
		: m_id(a_state.m_id)
		, m_role(a_state.m_role)
		, m_exposedAmount(a_exposedAmount)
	{
	}

	unsigned char GetId() const
	{
		return m_id;
	}

	Role GetRole() const
	{
		return m_role;
	}

private:

	unsigned char m_id;
	Role m_role;

};
