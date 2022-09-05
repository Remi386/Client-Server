#pragma once
#include <cstdint>
#include <tuple>
#include "../NetCommon/NetCommon.h"

class ClientInfo {
public:

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

		default:
			break;
		}
		
	}

	std::tuple<int64_t, int64_t> getBalance() const
	{
		return std::make_tuple(dollars, rubles);
	}

private:
	int64_t dollars = 0;
	int64_t rubles = 0;
};