#pragma once
#include <unordered_map>
#include <list>
#include "ClientInfo.h"
#include "CompletedTradeRequest.h"
#include <utility>
//#include "Session.h"
//#include <memory>

class DataBase {
public:

	DataBase(const DataBase&) = delete;
	DataBase(DataBase&&) = delete;
	void operator=(const DataBase&) = delete;
	void operator=(DataBase&&) = delete;

	static DataBase& instance() 
	{
		static DataBase db;
		return db;
	}

	int64_t registerNewUser() 
	{
		static int64_t counter = 0;
		clients.emplace(counter, ClientInfo());
		return counter++;
	}

	ClientInfo& getClientInfo(int64_t clientID)
	{
		return clients[clientID];
	}

	const std::list<CompletedTradeRequest>& 
		getClientTradeHistory(int64_t clientID) 
	{
		return tradeHistory[clientID];
	}

	void AddCompletedTradeRequest(int64_t clientID, const CompletedTradeRequest& request)
	{
		tradeHistory[clientID].push_back(request);
	}

private:
	DataBase() = default;

	std::unordered_map<int64_t, ClientInfo> clients;
	std::unordered_map<int64_t, std::list<CompletedTradeRequest>> tradeHistory;
	//std::unordered_map<int64_t, std::weak_ptr<Session>> activeSessions;
};