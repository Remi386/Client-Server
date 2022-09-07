#include "../Server/ClientInfo.h"
#include <gtest/gtest.h>

TEST(ClientInfoTest, ConstructorTest)
{
	ClientInfo balance1;
	EXPECT_EQ(balance1.getDollars(), 0);
	EXPECT_EQ(balance1.getRubles(), 0);

	ClientInfo balance2(30);
	EXPECT_EQ(balance2.getDollars(), 30);
	EXPECT_EQ(balance2.getRubles(), 0);

	ClientInfo balance3(30, 50);
	EXPECT_EQ(balance3.getDollars(), 30);
	EXPECT_EQ(balance3.getRubles(), 50);
}

TEST(ClientInfoTest, BalanceChangesTest)
{
	ClientInfo balance;
	balance.applyBalanceChanges(10, 50, TradeRequestType::Buy);

	auto [dollars1, rubles1] = balance.getBalance();
	EXPECT_EQ(dollars1, 10);
	EXPECT_EQ(rubles1, -50);

	balance.applyBalanceChanges(500, 4000, TradeRequestType::Sell);

	auto [dollars2, rubles2] = balance.getBalance();
	EXPECT_EQ(dollars2, -490);
	EXPECT_EQ(rubles2, 3950);
}