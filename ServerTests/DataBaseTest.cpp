#include "DataBaseTest.h"

TEST_F(DataBaseTest, RegisterNewUserTest)
{
	int64_t userID1 = database->registerNewUser("user1", "pass1");
	EXPECT_GT(userID1, 0) << "Incorrect start ID";

	int64_t userID2 = database->registerNewUser("user2", "pass2");
	EXPECT_EQ(userID2, userID1 + 1) << "Incorrect ID incrementation";

	int64_t userID3 = database->registerNewUser("user1", "pass3");
	EXPECT_EQ(userID3, -1) << "Incorrect handle of taken login";
}

TEST_F(DataBaseTest, GetUserIDTest)
{
	//Fill database
	int64_t userID1 = database->registerNewUser("user1", "pass1");
	int64_t userID2 = database->registerNewUser("user2", "pass2");
	int64_t userID3 = database->registerNewUser("user3", "pass3");

	//Method tests
	int64_t checkID1 = database->getUserID("user3", "pass3");
	EXPECT_EQ(checkID1, userID3) << "Cant get ID by correct ID and password";

	int64_t checkID2 = database->getUserID("user4", "pass1");
	EXPECT_EQ(checkID2, -1) << "Wrong handling of incorrect login";

	int64_t checkID3 = database->getUserID("user2", "pass3");
	EXPECT_EQ(checkID3, -2) << "Wrong handling of incorrect password";
}

TEST_F(DataBaseTest, GetClientInfoTest)
{
	int64_t userID1 = database->registerNewUser("user1", "pass1");
	
	//Get info of existing user
	std::optional<ClientInfo> firstUserBalance = 
		database->getClientInfo(userID1);
	
	//std::optional is convertable to bool
	ASSERT_TRUE(firstUserBalance);

	auto [dollars, rubles] = firstUserBalance->getBalance();

	EXPECT_EQ(dollars, 0);
	EXPECT_EQ(rubles, 0);

	//Get info of non-existing user
	std::optional<ClientInfo> secondUserBalance = 
		database->getClientInfo(345465);

	EXPECT_FALSE(secondUserBalance); //Info not found
}

TEST_F(DataBaseTest, UpdateClientInfoTest)
{
	int64_t userID1 = database->registerNewUser("user1", "pass1");
	int64_t userID2 = database->registerNewUser("user2", "pass2");

	///////////////First case//////////////////////////
	//Get info of existing user
	std::optional<ClientInfo> firstUserBalance1 = 
		database->getClientInfo(userID1);

	ASSERT_TRUE(firstUserBalance1);

	firstUserBalance1->applyBalanceChanges(30, 50, TradeRequestType::Buy);
	
	//Update information
	database->updateClientInfo(userID1, *firstUserBalance1);

	//Get information again
	std::optional<ClientInfo> firstUserBalance2 =
		database->getClientInfo(userID1);

	auto [dollars1, rubles1] = firstUserBalance2->getBalance();
	//Check if information written correctly
	EXPECT_EQ(dollars1, 30);
	EXPECT_EQ(rubles1, -50);


	///////////////Second case//////////////////////////
	std::optional<ClientInfo> secondUserBalance1 =
		database->getClientInfo(userID2);

	ASSERT_TRUE(secondUserBalance1);

	secondUserBalance1->applyBalanceChanges(48, 570, TradeRequestType::Sell);

	//Update information
	database->updateClientInfo(userID2, *secondUserBalance1);

	//Get information again
	std::optional<ClientInfo> secondUserBalance2 =
		database->getClientInfo(userID2);

	auto [dollars2, rubles2] = secondUserBalance2->getBalance();
	
	//Check if information written correctly
	EXPECT_EQ(dollars2, -48);
	EXPECT_EQ(rubles2, 570);
}

TEST_F(DataBaseTest, GetClientTradeHistoryTest)
{
	//Get trade history of user with ID = 50
	auto clientHistory1 = database->getClientTradeHistory(50);

	EXPECT_TRUE(clientHistory1.empty()) << "Failed to get empty history";

	//Register new users
	int64_t userID = database->registerNewUser("TradeHistoryUser1", "pass1");
	int64_t firstReqPatnerID = database->registerNewUser("TradeHistoryUser2", "pass1");;
	int64_t secondReqPatnerID = database->registerNewUser("TradeHistoryUser3", "pass1");;

	using boost::posix_time::ptime;
	using boost::posix_time::second_clock;

	//Fill his history
	ptime firstReqCreationTime = second_clock::universal_time();
	int64_t firstReqVolume = 50;
	int64_t firstReqPrice = 45;
	TradeRequestType firstReqType = TradeRequestType::Buy;

	ptime secondReqCreationTime = firstReqCreationTime + boost::posix_time::seconds(500);
	int64_t secondReqVolume = 300;
	int64_t secondReqPrice = 89;
	TradeRequestType secondReqType = TradeRequestType::Sell;

	//Create info about completed trade requests
	CompletedTradeRequest req1(firstReqVolume, firstReqPrice, 
							   firstReqPatnerID, firstReqCreationTime, 
							   firstReqType);

	CompletedTradeRequest req2(secondReqVolume, secondReqPrice, 
							   secondReqPatnerID, secondReqCreationTime, 
							   secondReqType);

	ptime firstReqCompletionTime = req1.getComletionTime();
	ptime secondReqCompletionTime = req2.getComletionTime();

	//Add it to user 
	database->addCompletedTradeRequest(userID, req1);
	database->addCompletedTradeRequest(userID, req2);

	//Run transaction to database
	auto clientHistory = database->getClientTradeHistory(userID);
	
	ASSERT_EQ(clientHistory.size(), 2) << "Some of the request missing or there are extra of them";

	//Unpacking data
	auto iter = clientHistory.begin();

	//First request 
	ptime creationTime1 = iter->getRegistrationTime();
	ptime completionTime1 = iter->getComletionTime();
	int64_t volume1 = iter->getVolume();
	int64_t price1 = iter->getPrice();
	int64_t otherID1 = iter->getOtherUserID();
	TradeRequestType type1 = iter->getType();

	EXPECT_EQ(firstReqVolume, volume1);
	EXPECT_EQ(firstReqPrice, price1);
	EXPECT_EQ(firstReqPatnerID, otherID1);
	EXPECT_EQ(firstReqType, type1);
	EXPECT_EQ(firstReqCreationTime, creationTime1);
	EXPECT_EQ(firstReqCompletionTime, completionTime1);

	//Second request
	++iter;

	ptime creationTime2 = iter->getRegistrationTime();
	ptime completionTime2 = iter->getComletionTime();
	int64_t volume2 = iter->getVolume();
	int64_t price2 = iter->getPrice();
	int64_t otherID2 = iter->getOtherUserID();
	TradeRequestType type2 = iter->getType();

	EXPECT_EQ(secondReqVolume, volume2);
	EXPECT_EQ(secondReqPrice, price2);
	EXPECT_EQ(secondReqPatnerID, otherID2);
	EXPECT_EQ(secondReqType, type2);
	EXPECT_EQ(secondReqCreationTime, creationTime2);
	EXPECT_EQ(secondReqCompletionTime, completionTime2);
}