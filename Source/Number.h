#pragma once

#include <cassert>
#include <csignal>
#include <limits>
#include <numeric>

namespace {

	template<typename Lhs, typename Rhs>
	Lhs _OverflowPreventedSum(Lhs a_lhs, Rhs a_rhs) {
		if (static_cast<Lhs>(a_rhs) >= std::numeric_limits<Lhs>::max() - a_lhs) {
			assert(false);
		}
		return a_lhs + a_rhs;
	}

	template<typename Lhs, typename Rhs>
	Lhs _OverflowPreventedMultiplication(Lhs a_lhs, Rhs a_rhs) {
		if (static_cast<Lhs>(a_rhs) >= std::numeric_limits<Lhs>::max() / a_lhs) {
			assert(false);
		}
		return a_lhs * a_rhs;
	}

}

struct Number
{
	long long divident;
	unsigned long long divisor;

	Number() = delete;

	Number(int a_value)
		: divident(a_value)
		, divisor(1)
	{
	}

	Number& operator/=(int a_rhs) {
		if (a_rhs != 1)
		{
			if (a_rhs < 0) {
				divident = -divident;
				divisor *= -a_rhs;
			} else {
				divisor *= a_rhs;
			}
			Simplify();
		}
		return *this;
	}

	Number& operator+=(Number a_rhs) {
		if (divident == 0)
		{
			divident = a_rhs.divident;
			divisor = a_rhs.divisor;
		}
		else
		{
			if (divisor == a_rhs.divisor)
			{
				divident = _OverflowPreventedSum(divident, a_rhs.divident);
			}
			else
			{
				divident = _OverflowPreventedSum(_OverflowPreventedMultiplication(divident, a_rhs.divisor), _OverflowPreventedMultiplication(a_rhs.divident, divisor));
				divisor = _OverflowPreventedMultiplication(divisor, a_rhs.divisor);
			}
			Simplify();
		}
		return *this;
	}

private:

	unsigned long long GetGcd() const
	{
		return std::gcd(divident, divisor);
	}

	void Simplify()
	{
		const auto gcd{GetGcd()};
		divident /= gcd;
		divisor /= gcd;
	}

};
