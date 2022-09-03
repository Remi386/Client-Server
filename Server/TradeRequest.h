#pragma once
#include <cstdint>
#include <string>

class TradeRequest {
public:
	enum class Type : uint8_t {
		Buy,
		Sell
	};

	TradeRequest(int64_t owner_, int64_t volume_, int64_t price_, Type type_)
		: owner(owner_), volume(volume_), price(price_), type(type_)
	{}

	int64_t getOwner() const { return owner; }

	int64_t getVolume() const { return volume; }

	int64_t getPrice() const { return price; }

	Type getType() const { return type; }

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
		return std::string((type == TradeRequest::Type::Buy ? "Buying " : "Selling ")
						   + std::to_string(volume) + " dollars with "
						   + std::to_string(price)  + " rubles price.");
	}

private:
	int64_t owner;
	mutable int64_t volume;
	int64_t price;
	Type type;
};

class TradeRequestDescendingComparator {
public:
	bool operator()(const TradeRequest& lhs, const TradeRequest& rhs) const
	{
		return lhs.getPrice() < rhs.getPrice();
	}
};

class TradeRequestAscendingComparator {
public:
	bool operator()(const TradeRequest& lhs, const TradeRequest& rhs) const
	{
		return lhs.getPrice() > rhs.getPrice();
	}
};