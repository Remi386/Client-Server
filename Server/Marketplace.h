#pragma once
#include <set>
#include <list>
#include <map>
#include "TradeRequest.h"

class Marketplace {
public:
	using listOfRequests = std::list<const TradeRequest*>;

	Marketplace(const Marketplace&) = delete;
	Marketplace(Marketplace&&) = delete;
	void operator=(const Marketplace&) = delete;
	void operator=(Marketplace&&) = delete;

	static Marketplace& instance()
	{
		static Marketplace market;
		return market;
	}

	void handleTradeRequest(TradeRequest& tradeRequest);

	const listOfRequests& getActiveRequests(int64_t userID);

private:

	void addRequest(TradeRequest& req, TradeRequest::Type reqType);

	void removeRequest(const TradeRequest& req, TradeRequest::Type reqType);

	Marketplace() = default;

	std::multiset<TradeRequest, TradeRequestAscendingComparator> buyRequests;
	std::multiset<TradeRequest, TradeRequestDescendingComparator> sellRequests;
	std::map<int64_t, listOfRequests> activeRequests;
};