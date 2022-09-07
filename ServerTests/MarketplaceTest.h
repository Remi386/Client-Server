#pragma once
#include <gtest/gtest.h>
#include "../Server/Marketplace.h"
#include "../Server/DataBase.h"

class MarketplaceTest : public testing::Test {
protected:

	virtual void SetUp() override
	{
		database = new DataBase();
		market = new Marketplace(*database);
	}

	virtual void TearDown() override
	{
		delete market;
		delete database;
	}

	bool isTradeEqual(const TradeRequest& lhs, const TradeRequest& rhs)
	{
		return lhs.getOwner() == rhs.getOwner()
			&& lhs.getPrice() == rhs.getPrice()
			&& lhs.getRegistrationTime() == rhs.getRegistrationTime()
			&& lhs.getVolume() == rhs.getVolume()
			&& lhs.getType() == rhs.getType();
	}

	bool isInfoEqual(const ClientInfo& lhs, const ClientInfo& rhs)
	{
		return lhs.getRubles() == rhs.getRubles()
			&& lhs.getDollars() == rhs.getDollars();
	}
	
	void RegisterUsers(size_t count)
	{
		for (size_t i = 0; i < count; ++i) {
			database->registerNewUser("user" + std::to_string(i),
									  "pass" + std::to_string(i));
		}
	}

	DataBase* database;
	Marketplace* market;
};