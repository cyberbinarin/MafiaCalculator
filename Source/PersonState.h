#pragma once

#include <memory>
#include "Roles.h"

class PersonState {
public:
	const unsigned char m_id;
	const Role m_role;
	bool m_exposed{false};

	PersonState(unsigned char a_id, Role a_role)
		: m_id(a_id)
		, m_role(a_role)
	{
	}

	virtual std::shared_ptr<PersonState> CreateExposedCopy() const
	{
		auto result{std::make_shared<PersonState>(*this)};
		result->m_exposed = true;
		return result;
	}

};
