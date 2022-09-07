#pragma once
#include <unordered_map>
#include <list>
#include <string>
#include "ClientInfo.h"
#include "CompletedTradeRequest.h"
#include <utility>
#include <memory>
#include <optional>

class Session;

class DataBase {
public:

	DataBase() = default;

	DataBase(const DataBase&) = delete;
	DataBase(DataBase&&) = delete;
	void operator=(const DataBase&) = delete;
	void operator=(DataBase&&) = delete;

	/// <summary>
	/// Register new user in database
	/// </summary>
	/// <param name="login"></param>
	/// <returns> New user's ID if login not taken, -1 otherwise </returns>
	int64_t registerNewUser(const std::string& login, const std::string& password);

	/// <summary>
	/// Check for user existence and password correctness
	/// </summary>
	/// <param name="login"></param>
	/// <param name="password"></param>
	/// <returns> Positive userID on success, -1 if user not found, -2 if
	/// passwords mismatch </returns>
	int64_t getUserID(const std::string& login, const std::string& password) const;

	/// <summary>
	/// Get client's balance information.
	/// </summary>
	/// <param name="clientID"></param>
	/// <returns>If user doesnt exist, return null optional object</returns>
	std::optional<ClientInfo> getClientInfo(int64_t clientID)
	{
		if(clientsInfo.find(clientID) != clientsInfo.end())
			return clientsInfo[clientID];

		return std::nullopt;
	}

	/// <summary>
	/// Updates client's balance. Does nothing, if user doesn't exist
	/// </summary>
	/// <param name="clientID"></param>
	/// <param name="newInfo"></param>
	void updateClientInfo(int64_t clientID, const ClientInfo& newInfo)
	{
		if (clientsInfo.find(clientID) != clientsInfo.end())
			clientsInfo[clientID] = newInfo;
	}

	/// <summary>
	/// Get client's trade history by ID
	/// </summary>
	/// <param name="clientID"></param>
	/// <returns> List of completed requests, which can be empty </returns>
	const std::list<CompletedTradeRequest>& 
		getClientTradeHistory(int64_t clientID) 
	{
		return tradeHistory[clientID];
	}

	/// <summary>
	/// Write trade request completion to history
	/// </summary>
	/// <param name="clientID"></param>
	/// <param name="request"></param>
	void addCompletedTradeRequest(int64_t clientID, const CompletedTradeRequest& request)
	{
		tradeHistory[clientID].push_back(request);
	}

private:
	std::unordered_map<std::string, int64_t> login2ID;
	std::unordered_map<int64_t, std::string> ID2Password;

	std::unordered_map<int64_t, ClientInfo> clientsInfo;
	std::unordered_map<int64_t, std::list<CompletedTradeRequest>> tradeHistory;
};