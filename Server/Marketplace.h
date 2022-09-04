#pragma once
#include <set>
#include <list>
#include <unordered_map>
#include "TradeRequest.h"
#include <memory>

class Session;

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

	std::weak_ptr<Session> getSession(int64_t userID)
	{
		if (activeSessions.find(userID) != activeSessions.end())
			return activeSessions[userID];

		return std::weak_ptr<Session>();
	}

	void addSession(int64_t userID, std::weak_ptr<Session> session)
	{
		if (activeSessions.find(userID) == activeSessions.end())
			activeSessions.emplace(userID, session);
	}

	void removeSession(int64_t userID)
	{
		if (activeSessions.find(userID) != activeSessions.end())
			activeSessions.erase(userID);
	}

private:

	void addRequest(TradeRequest& req, TradeRequest::Type reqType);

	void removeRequest(const TradeRequest& req, TradeRequest::Type reqType);

	Marketplace() = default;

	std::multiset<TradeRequest, TradeRequestAscendingComparator> buyRequests;
	std::multiset<TradeRequest, TradeRequestDescendingComparator> sellRequests;
	
	std::unordered_map<int64_t, listOfRequests> activeRequests;
	std::unordered_map<int64_t, std::weak_ptr<Session>> activeSessions;
};