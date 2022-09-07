#include "MarketplaceTest.h"
#include "../Server/Session.h"
#include <random>

TEST_F(MarketplaceTest, GetActiveRequestsTest)
{
	//Get empty history:
	auto& emptyRequests = market->getActiveRequests(34563);

	ASSERT_TRUE(emptyRequests.empty());

	//User ID = 5, volume = 40, price = 78
	int64_t firstUserID = 5;
	int64_t secondUserID = 13;
	TradeRequest req1(firstUserID, 40, 78, TradeRequestType::Buy);
	TradeRequest req2(secondUserID, 24, 921, TradeRequestType::Buy);
	
	//Market should add requests to active for this users
	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);

	//Check first user requests
	auto& userRequests1 = market->getActiveRequests(firstUserID);

	ASSERT_FALSE(userRequests1.empty());

	auto reqAct1 = userRequests1.front();
	EXPECT_TRUE(isTradeEqual(req1, *reqAct1));

	//Check second user requests
	auto& userRequests2 = market->getActiveRequests(secondUserID);

	ASSERT_FALSE(userRequests2.empty());

	auto reqAct2 = userRequests2.front();
	EXPECT_TRUE(isTradeEqual(req2, *reqAct2));
}

TEST_F(MarketplaceTest, AddSessionTest)
{
	boost::asio::io_context context;

	//Create fictive session
	std::shared_ptr<Session> session
		= std::make_shared<Session>(context, *market, *database);

	market->addSession(45, session->shared_from_this());

	//Get existing session
	std::weak_ptr<Session> sess1 = market->getSession(45);

	EXPECT_TRUE(sess1.lock());

	//Get non-existing session
	std::weak_ptr<Session> sess2 = market->getSession(12345);

	EXPECT_EQ(sess2.lock(), std::weak_ptr<Session>().lock());
}

TEST_F(MarketplaceTest, RemoveSessionTest)
{
	int64_t userID = 45;
	boost::asio::io_context context;

	//Create fictive session
	std::shared_ptr<Session> session
		= std::make_shared<Session>(context, *market, *database);

	market->addSession(userID, session->shared_from_this());

	//Get existing session
	std::weak_ptr<Session> sess1 = market->getSession(userID);

	EXPECT_TRUE(sess1.lock());

	market->removeSession(userID);

	std::weak_ptr<Session> sess2 = market->getSession(userID);

	//Session should not exist anymore
	EXPECT_EQ(sess2.lock(), std::weak_ptr<Session>().lock());
}

TEST_F(MarketplaceTest, CancelRequestTest)
{
	int64_t userID = 4;
	TradeRequest req1(userID, 56, 70, TradeRequestType::Buy);

	//Register request
	market->handleTradeRequest(req1);

	auto& activeReqs1 = market->getActiveRequests(userID);
	auto& buyReqs1 = market->getBuyRequests();

	ASSERT_FALSE(activeReqs1.empty());
	ASSERT_FALSE(buyReqs1.empty());

	EXPECT_TRUE(isTradeEqual(req1, *activeReqs1.front()));
	EXPECT_TRUE(isTradeEqual(req1, *buyReqs1.begin()));

	//Remove random request
	EXPECT_FALSE(market->removeRequest(TradeRequest(userID, 0, 0,
		TradeRequestType::Buy)));

	//Create another request with simular parameters
	TradeRequest req2(userID, 56, 70, TradeRequestType::Buy);

	//Remove request
	EXPECT_TRUE(market->removeRequest(req2));

	auto& activeReqs2 = market->getActiveRequests(userID);
	auto& buyReqs2 = market->getBuyRequests();

	ASSERT_TRUE(activeReqs2.empty());
	ASSERT_TRUE(buyReqs2.empty());
}

TEST_F(MarketplaceTest, BuyRequestsOrderTest)
{
	using boost::posix_time::ptime;
	using boost::posix_time::second_clock;
	using boost::posix_time::seconds;

	ptime timeOfCreation = second_clock::universal_time();

	//Highest price, it should be first
	TradeRequest req1(2, 10, 78, TradeRequestType::Buy);

	//Earliest creation time, it should be second
	TradeRequest req2(3, 10, 40, timeOfCreation, TradeRequestType::Buy);

	//Latest creation time, it should be third
	TradeRequest req3(4, 10, 40, timeOfCreation + seconds(2), TradeRequestType::Buy);

	//Lowest price, it should be last
	TradeRequest req4(1, 10, 30, TradeRequestType::Buy);

	std::vector<TradeRequest> requests = { req1, req2, req3, req4 };

	//Make marketplace handle it in random order
	std::shuffle(requests.begin(), requests.end(),
		std::default_random_engine());

	for (auto& req : requests) {
		market->handleTradeRequest(req);
	}

	auto& buyRequest = market->getBuyRequests();

	ASSERT_EQ(buyRequest.size(), 4);

	auto iter = buyRequest.begin();

	EXPECT_TRUE(isTradeEqual(*iter++, req1));
	EXPECT_TRUE(isTradeEqual(*iter++, req2));
	EXPECT_TRUE(isTradeEqual(*iter++, req3));
	EXPECT_TRUE(isTradeEqual(*iter++, req4));
}

TEST_F(MarketplaceTest, SellRequestsOrderTest)
{
	using boost::posix_time::ptime;
	using boost::posix_time::second_clock;
	using boost::posix_time::seconds;

	ptime timeOfCreation = second_clock::universal_time();

	//Lowest price, it should be first
	TradeRequest req1(2, 10, 30, TradeRequestType::Sell);

	//Earliest creation time, it should be second
	TradeRequest req2(3, 10, 40, timeOfCreation, TradeRequestType::Sell);

	//Latest creation time, it should be third
	TradeRequest req3(4, 10, 40, timeOfCreation + seconds(2), TradeRequestType::Sell);

	//Highest price, it should be last
	TradeRequest req4(1, 10, 78, TradeRequestType::Sell);

	std::vector<TradeRequest> requests = { req1, req2, req3, req4 };

	//Make marketplace handle it in random order
	std::shuffle(requests.begin(), requests.end(),
		std::default_random_engine());

	for (auto& req : requests) {
		market->handleTradeRequest(req);
	}

	auto& sellRequest = market->getSellRequests();

	ASSERT_EQ(sellRequest.size(), 4);

	auto iter = sellRequest.begin();

	EXPECT_TRUE(isTradeEqual(*iter++, req1));
	EXPECT_TRUE(isTradeEqual(*iter++, req2));
	EXPECT_TRUE(isTradeEqual(*iter++, req3));
	EXPECT_TRUE(isTradeEqual(*iter++, req4));
}

//First user buy 50 USD with 60 rubles price
//Second user sell 30 USD with 75 rubles price
//Expectations: 
//	user1 balance = +30 usd and -1800 rubles, 
//		with 20 USD and 60 rubles price (buy)
//	user2 balance = -30 usd and 1800 rubles, no active request 
TEST_F(MarketplaceTest, FirstTestCase)
{
	int64_t user1 = database->registerNewUser("1CaseUser1", "1CasePass1");
	int64_t user2 = database->registerNewUser("1CaseUser2", "1CasePass2");

	TradeRequest req1(user1, 50, 60, TradeRequestType::Buy);
	TradeRequest req2(user2, 30, 75, TradeRequestType::Sell);

	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user1).value(),
							ClientInfo(30, -1800)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user2).value(),
							ClientInfo(-30, 1800)));

	auto& buyRequests = market->getBuyRequests();

	ASSERT_FALSE(buyRequests.empty());

	TradeRequest result(user1, 20, 60, TradeRequestType::Buy);

	EXPECT_TRUE(isTradeEqual(result, *buyRequests.begin()));
}

//First user buy 70 USD with 56 rubles price
//Second user sell 120 USD with 89 rubles price
//Expectations: 
//	user1 balance = +70 usd and -3920 rubles, no active requests
//	user2 balance = -70 usd and 3920 rubles, 
//		active request with 50 USD and 89 rubles price (sell)
TEST_F(MarketplaceTest, SecondTestCase)
{
	int64_t user1 = database->registerNewUser("2CaseUser1", "2CasePass1");
	int64_t user2 = database->registerNewUser("2CaseUser2", "2CasePass2");

	TradeRequest req1(user1, 70, 56, TradeRequestType::Buy);
	TradeRequest req2(user2, 120, 89, TradeRequestType::Sell);

	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user1).value(),
							ClientInfo(70, -3920)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user2).value(),
							ClientInfo(-70, 3920)));

	auto& sellRequests = market->getSellRequests();

	ASSERT_FALSE(sellRequests.empty());

	TradeRequest result(user2, 50, 89, TradeRequestType::Sell);

	EXPECT_TRUE(isTradeEqual(result, *sellRequests.begin()));
}


//First user buy 10 USD with 62 rubles price
//Second user buy 20 USD with 63 rubles price
//Third user sell 50 USD with 61 rubles price
//Expectations: 
//	user1 balance = +10 USD and -620 rubles, no active requests
//	user2 balance = +20 USD and -1260 rubles, no active requests
//	user3 balance = -30 USD and +1880 rubles
//		active request with 50 USD and 89 rubles price (sell)
TEST_F(MarketplaceTest, ThirdTestCase)
{
	int64_t user1 = database->registerNewUser("3CaseUser1", "3CasePass1");
	int64_t user2 = database->registerNewUser("3CaseUser2", "3CasePass2");
	int64_t user3 = database->registerNewUser("3CaseUser3", "3CasePass3");

	TradeRequest req1(user1, 10, 62, TradeRequestType::Buy);
	TradeRequest req2(user2, 20, 63, TradeRequestType::Buy);
	TradeRequest req3(user3, 50, 61, TradeRequestType::Sell);

	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);
	market->handleTradeRequest(req3);

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user1).value(),
							ClientInfo(10, -620)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user2).value(),
							ClientInfo(20, -1260)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user3).value(),
							ClientInfo(-30, 1880)));

	auto& sellRequests = market->getSellRequests();

	ASSERT_FALSE(sellRequests.empty());

	TradeRequest result(user3, 20, 61, TradeRequestType::Sell);

	EXPECT_TRUE(isTradeEqual(result, *sellRequests.begin()));
}

//First user buy 40 USD with 55 rubles price
//Second user buy 70 USD with 54 rubles price
//Third user sell 60 USD with 67 rubles price
//Expectations: 
//	user1 balance = +40 USD and -2200 rubles, no active requests
//	user2 balance = +20 USD and -1080 rubles, 
//		active request with 50 USD and 54 rubles price (buy)
//	user3 balance = -60 USD and +3280 rubles no active requests
TEST_F(MarketplaceTest, FourthTestCase)
{
	int64_t user1 = database->registerNewUser("4CaseUser1", "4CasePass1");
	int64_t user2 = database->registerNewUser("4CaseUser2", "4CasePass2");
	int64_t user3 = database->registerNewUser("4CaseUser3", "4CasePass3");

	TradeRequest req1(user1, 40, 55, TradeRequestType::Buy);
	TradeRequest req2(user2, 70, 54, TradeRequestType::Buy);
	TradeRequest req3(user3, 60, 67, TradeRequestType::Sell);

	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);
	market->handleTradeRequest(req3);

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user1).value(),
							ClientInfo(40, -2200)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user2).value(),
							ClientInfo(20, -1080)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user3).value(),
							ClientInfo(-60, 3280)));

	auto& buyRequests = market->getBuyRequests();

	ASSERT_FALSE(buyRequests.empty());

	TradeRequest result(user2, 50, 54, TradeRequestType::Buy);

	EXPECT_TRUE(isTradeEqual(result, *buyRequests.begin()));
}

//First user sell 50 USD with 62 rubles price
//Second user sell 30 USD with 59 rubles price
//Third user buy 70 USD with 61 rubles price
//Expectations: 
//	user1 balance = -40 USD and +2480 rubles,
//		active request with 10 USD and 62 rubles price (sell)
//	user2 balance = -30 USD and +1770 rubles, no active requests
//	user3 balance = +70 USD and -4250 rubles  no active requests
TEST_F(MarketplaceTest, FifthTestCase)
{
	int64_t user1 = database->registerNewUser("5CaseUser1", "5CasePass1");
	int64_t user2 = database->registerNewUser("5CaseUser2", "5CasePass2");
	int64_t user3 = database->registerNewUser("5CaseUser3", "5CasePass3");

	TradeRequest req1(user1, 50, 62, TradeRequestType::Sell);
	TradeRequest req2(user2, 30, 59, TradeRequestType::Sell);
	TradeRequest req3(user3, 70, 61, TradeRequestType::Buy);

	market->handleTradeRequest(req1);
	market->handleTradeRequest(req2);
	market->handleTradeRequest(req3);

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user1).value(),
							ClientInfo(-40, 2480)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user2).value(),
							ClientInfo(-30, 1770)));

	EXPECT_TRUE(isInfoEqual(database->getClientInfo(user3).value(),
						  ClientInfo(70, -4250)));

	auto& sellRequests = market->getSellRequests();

	ASSERT_FALSE(sellRequests.empty());

	TradeRequest result(user1, 10, 62, TradeRequestType::Sell);

	EXPECT_TRUE(isTradeEqual(result, *sellRequests.begin()));
}