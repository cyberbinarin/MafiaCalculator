#pragma once

#include <set>
#include "PersonState.h"

class DetectiveState : public PersonState
{
public:
	//Which people this detective has inspected, by id
	std::set<unsigned char> m_inspectedPeople;

	//if true, the detective has announced to everyone that he is a detective and everyone belived him
	bool m_hasExposedHimself{false};

	DetectiveState(unsigned char a_id)
		: PersonState(a_id, Role::Detective)
	{
	}

	virtual std::shared_ptr<PersonState> CreateExposedCopy() const override
	{
		auto result{std::make_shared<DetectiveState>(*this)};
		result->m_exposed = true;
		return result;
	}

	virtual std::shared_ptr<PersonState> CreateCopyAndInspect(unsigned char a_inspectedPersonId) const
	{
		auto result{std::make_shared<DetectiveState>(*this)};
		result->m_inspectedPeople.insert(a_inspectedPersonId);
		return result;
	}


};
