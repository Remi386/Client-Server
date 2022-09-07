#pragma once
#include <cstdint>
#include <tuple>
#include "../NetCommon/NetCommon.h"

class ClientInfo {
public:

	explicit ClientInfo(int64_t dollars_ = 0, int64_t rubles_ = 0)
		:dollars(dollars_), rubles(rubles_)
	{}

	void applyBalanceChanges(int64_t dollChange, int64_t rubChange, TradeRequestType type)
	{
		switch (type)
		{
		case TradeRequestType::Buy:
			dollars += dollChange;
			rubles -= rubChange;
			break;

		case TradeRequestType::Sell:
			dollars -= dollChange;
			rubles += rubChange;
			break;
		}
	}

	int64_t getDollars() const { return dollars; }

	int64_t getRubles() const { return rubles; }

	std::tuple<int64_t, int64_t> getBalance() const
	{
		return std::make_tuple(dollars, rubles);
	}

private:
	int64_t dollars = 0;
	int64_t rubles = 0;
};