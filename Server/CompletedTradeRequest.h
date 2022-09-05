#pragma once
#include <cstdint>
#include "../NetCommon/TradeRequest.h"
#include <string>

class CompletedTradeRequest {
public:
	using second_clock = boost::date_time::second_clock<boost::posix_time::ptime>;

	CompletedTradeRequest(int64_t volume_, int64_t price_, 
						  int64_t otherUserID_, 
						  const boost::posix_time::ptime& registrationTime,
						  TradeRequestType type_)
		: volume(volume_), 
		price(price_), 
		otherUserID(otherUserID_), 
		timeOfRegistration(registrationTime),
		timeOfCompletion(second_clock::universal_time()),
		type(type_)
	{}

	int64_t getVolume() const { return volume; }

	int64_t getPrice() const { return price; }

	int64_t getOtherUserID() const { return otherUserID; }

	TradeRequestType getType() const { return type; }

	std::string toString() const;

	const boost::posix_time::ptime& getRegistrationTime() const
	{
		return timeOfRegistration;
	}

	const boost::posix_time::ptime& getComletionTime() const
	{
		return timeOfCompletion;
	}

	nlohmann::json createJsonObject() const;

private:
	int64_t volume;
	int64_t price;
	int64_t otherUserID;
	
	boost::posix_time::ptime timeOfRegistration;
	boost::posix_time::ptime timeOfCompletion;

	TradeRequestType type;
};