#pragma once
#include <unordered_map>
#include <list>
#include <string>
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

	/// <summary>
	/// Register new user in database
	/// </summary>
	/// <param name="login"></param>
	/// <returns> Return new user's ID if login not taken, -1 otherwise </returns>
	int64_t registerNewUser(const std::string& login, const std::string& password) 
	{
		static int64_t counter = 0;

		if (login2ID.find(login) == login2ID.end()) 
		{	
			clientsInfo.emplace(counter, ClientInfo());
			login2ID.emplace(login, counter);
			ID2Password.emplace(counter, password);

			return counter++;
		}

		return -1;
	}

	/// <summary>
	/// Check for user existence and password correctness
	/// </summary>
	/// <param name="login"></param>
	/// <param name="password"></param>
	/// <returns> Returns positive userID on success, -1 if user not found, -2 if
	/// passwords mismatch </returns>
	int64_t getUserID(const std::string& login, const std::string& password) const
	{
		auto iter = login2ID.find(login);

		if (iter != login2ID.end()) {
			if (ID2Password.at(iter->second) == password) {
				return iter->second;
			}

			return -2;
		}

		return -1;
	}

	ClientInfo& getClientInfo(int64_t clientID)
	{
		return clientsInfo[clientID];
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

	std::unordered_map<std::string, int64_t> login2ID;
	std::unordered_map<int64_t, std::string> ID2Password;

	std::unordered_map<int64_t, ClientInfo> clientsInfo;
	std::unordered_map<int64_t, std::list<CompletedTradeRequest>> tradeHistory;
	//std::unordered_map<int64_t, std::weak_ptr<Session>> activeSessions;
};