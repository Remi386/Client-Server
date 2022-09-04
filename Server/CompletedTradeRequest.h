#pragma once
#include <cstdint>
#include "TradeRequest.h"
#include <string>

class CompletedTradeRequest {
public:
	CompletedTradeRequest(int64_t volume_, int64_t price_, 
						  int64_t otherUserID_, TradeRequest::Type type_)
		: volume(volume_), price(price_), otherUserID(otherUserID_), type(type_)
	{}

	int64_t getVolume() const { return volume; }

	int64_t getPrice() const { return price; }

	int64_t getOtherUserID() const { return otherUserID; }

	TradeRequest::Type getType() const { return type; }

	std::string toString() const;

private:
	int64_t volume;
	int64_t price;
	int64_t otherUserID;
	TradeRequest::Type type;
};