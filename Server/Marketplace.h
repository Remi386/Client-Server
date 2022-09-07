#pragma once
#include <set>
#include <list>
#include <unordered_map>
#include "../NetCommon/TradeRequest.h"
#include <memory>

class Session;
class DataBase;

class Marketplace {
public:

	Marketplace(DataBase& database_)
		:database(database_)
	{}

	using listOfRequests = std::list<const TradeRequest*>;
	using buyRequestsContainer = std::multiset<TradeRequest, TradeRequestAscendingComparator>;
	using sellRequestsContainer = std::multiset<TradeRequest, TradeRequestDescendingComparator>;

	Marketplace(const Marketplace&) = delete;
	Marketplace(Marketplace&&) = delete;
	void operator=(const Marketplace&) = delete;
	void operator=(Marketplace&&) = delete;

	void handleTradeRequest(TradeRequest& tradeRequest);

	const listOfRequests& getActiveRequests(int64_t userID);

	/// <summary>
	/// Get active session with client by his ID
	/// </summary>
	/// <param name="userID"></param>
	/// <returns> weak_ptr with Session on success,
	/// empty weak_ptr on failure </returns>
	std::weak_ptr<Session> getSession(int64_t userID)
	{
		if (activeSessions.find(userID) != activeSessions.end())
			return activeSessions[userID];

		return std::weak_ptr<Session>();
	}

	/// <summary>
	/// Adds client's session to live sessions
	/// </summary>
	void addSession(int64_t userID, std::weak_ptr<Session> session)
	{
		if (activeSessions.find(userID) == activeSessions.end())
			activeSessions.emplace(userID, session);
	}

	/// <summary>
	/// Removes client's session from live session
	/// </summary>
	/// <param name="userID"></param>
	void removeSession(int64_t userID)
	{
		if (activeSessions.find(userID) != activeSessions.end())
			activeSessions.erase(userID);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="req"></param>
	 
	/// <summary>
	/// Remove request from active and request.Type container
	/// </summary>
	/// <param name="request">Request with parameters 
	/// simular to request which should be removed</param>
	/// <returns>True if operation succeed, false otherwise</returns>
	bool removeRequest(const TradeRequest& request);

	const sellRequestsContainer& getSellRequests() const { return sellRequests; }

	const buyRequestsContainer& getBuyRequests() const { return buyRequests; }

private:
	/// <summary>
	/// Add request to active and request.Type container
	/// </summary>
	/// <param name="request"></param>
	void addRequest(TradeRequest& request);

private:
	//Dependency injection
	DataBase& database;

	buyRequestsContainer buyRequests;
	sellRequestsContainer sellRequests;
	
	std::unordered_map<int64_t, listOfRequests> activeRequests;
	std::unordered_map<int64_t, std::weak_ptr<Session>> activeSessions;
};