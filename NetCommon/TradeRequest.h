#pragma once
#include <cstdint>
#include <string>
#include <boost/date_time.hpp>
#include <nlohmann/json.hpp>
#include "../NetCommon/NetCommon.h"

class TradeRequest {
public:

	using second_clock = boost::date_time::second_clock<boost::posix_time::ptime>;

	/// <summary>
	/// This constructor records creation time by itself
	/// </summary>
	TradeRequest(int64_t owner_, int64_t volume_, int64_t price_, TradeRequestType type_)
		: owner(owner_), 
		volume(volume_), 
		price(price_), 
		timeOfRegistration(second_clock::universal_time()),
		type(type_)
	{}

	/// <summary>
	/// This constructor takes a creation time
	/// </summary>
	TradeRequest(int64_t owner_, int64_t volume_, int64_t price_, 
				 boost::posix_time::ptime creationTime, TradeRequestType type_)
		: owner(owner_),
		volume(volume_),
		price(price_),
		timeOfRegistration(creationTime),
		type(type_)
	{}

	int64_t getOwner() const { return owner; }

	int64_t getVolume() const { return volume; }

	int64_t getPrice() const { return price; }

	TradeRequestType getType() const { return type; }

	const boost::posix_time::ptime& getRegistrationTime() const
	{
		return timeOfRegistration;
	}

	std::string getRegistrationTimeString() const
	{
		return boost::posix_time::to_iso_extended_string(timeOfRegistration);
	}

	bool isCompleted() const
	{
		return volume == 0;
	}

	void applyTrade(int64_t tradeVolume) const
	{
		volume -= tradeVolume;
	}

	void setVolume(int64_t newVolume) { volume = newVolume; }

	void setPrice(int64_t newPrice) { price = newPrice; }

	std::string toString() const
	{
		return std::string((type == TradeRequestType::Buy ? "Buying " : "Selling ")
			+ std::to_string(volume) + " dollars with "
			+ std::to_string(price) + " rubles price. Published on "
			+ getRegistrationTimeString());
	}

	nlohmann::json createJsonObject() const
	{
		nlohmann::json object;

		object["Price"] = price;
		object["Volume"] = volume;
		object["Type"] = type;
		object["UserID"] = owner;
		object["RegTime"] = getRegistrationTimeString();

		return object;
	}

private:
	int64_t owner;
	mutable int64_t volume;
	int64_t price;
	boost::posix_time::ptime timeOfRegistration;
	TradeRequestType type;
};

//Sorts trade requests by increasing price, 
//increasing registration time and increasing userID
class TradeRequestDescendingComparator {
public:
	bool operator()(const TradeRequest& lhs, const TradeRequest& rhs) const
	{
		if (lhs.getPrice() != rhs.getPrice()) {
			return lhs.getPrice() < rhs.getPrice();
		}
		else if (lhs.getRegistrationTime() != rhs.getRegistrationTime()) {
			lhs.getRegistrationTime() < rhs.getRegistrationTime();
		}
		else {
			return lhs.getOwner() < rhs.getOwner();
		}
	}
};

//Sorts trade requests by desreasing price, 
//increasing registration time and increasing userID
class TradeRequestAscendingComparator {
public:
	bool operator()(const TradeRequest& lhs, const TradeRequest& rhs) const
	{
		if (lhs.getPrice() != rhs.getPrice()) {
			return lhs.getPrice() > rhs.getPrice();
		}
		else if (lhs.getRegistrationTime() != rhs.getRegistrationTime()) {
			lhs.getRegistrationTime() < rhs.getRegistrationTime();
		}
		else {
			return lhs.getOwner() < rhs.getOwner();
		}
	}
};