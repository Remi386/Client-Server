#pragma once
#include <gtest/gtest.h>
#include "../Server/Session.h"
#include "../Server/DataBase.h"
#include "../Server/Marketplace.h"

class SessionTest : public testing::Test {
protected:

	virtual void SetUp() override
	{
		database = new DataBase();
		market = new Marketplace(*database);
		session = std::make_shared<Session>(context, *market, *database);
	}

	virtual void TearDown() override
	{
		session.~shared_ptr();
		delete market;
		delete database;
	}

	nlohmann::json createRequest(RequestType type, int64_t userID,
								 nlohmann::json message)
	{
		nlohmann::json request;

		request["RequestType"] = type;
		request["UserID"] = userID;
		request["Message"] = message;

		return request;
	}

	void checkResponse(nlohmann::json& response, ResponseType type,
					   bool status, nlohmann::json& supposedMessage)
	{
		EXPECT_EQ(response["ResponseType"].get<ResponseType>(), type);
		EXPECT_EQ(response["Status"].get<bool>(), status);

		for (auto& [key, value] : supposedMessage.items())
		{
			EXPECT_EQ(response["Message"][key], value);
		}
	}
	
	std::shared_ptr<Session> session;
	boost::asio::io_context context;
	DataBase* database;
	Marketplace* market;
};