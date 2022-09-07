#include "TradeRequestTest.h"

TEST_F(TradeRequestTest, ConstructorTest)
{
	TradeRequest req1(1, 2, 3, TradeRequestType::Buy);

	EXPECT_EQ(req1.getOwner(), 1);
	EXPECT_EQ(req1.getVolume(), 2);
	EXPECT_EQ(req1.getPrice(), 3);
	EXPECT_EQ(req1.getType(), TradeRequestType::Buy);


	boost::posix_time::ptime creationTime;
	TradeRequest req2(13, 45, 23, creationTime, TradeRequestType::Sell);

	EXPECT_EQ(req2.getOwner(), 13);
	EXPECT_EQ(req2.getVolume(), 45);
	EXPECT_EQ(req2.getPrice(), 23);
	EXPECT_EQ(req2.getRegistrationTime(), creationTime);
	EXPECT_EQ(req2.getType(), TradeRequestType::Sell);
}

TEST_F(TradeRequestTest, ComparatorTest)
{
	boost::posix_time::ptime creationTime = boost::posix_time::second_clock::universal_time();
	
	///////////////Test by price/////////////////////////
	TradeRequest req1(0, 0, 22, creationTime, TradeRequestType::Buy);
	TradeRequest req2(0, 0, 23, creationTime, TradeRequestType::Buy);
	
	EXPECT_TRUE(TradeRequestDescendingComparator()(req1, req2));
	EXPECT_FALSE(TradeRequestDescendingComparator()(req2, req1));

	EXPECT_FALSE(TradeRequestAscendingComparator()(req1, req2));
	EXPECT_TRUE(TradeRequestAscendingComparator()(req2, req1));

	boost::posix_time::ptime newTime = creationTime + boost::posix_time::seconds(1);
	EXPECT_LE(creationTime, newTime);

	///////////////Test by time of creation /////////////////////////
	TradeRequest req3(0, 0, 22, creationTime, TradeRequestType::Buy);

	TradeRequest req4(0, 0, 22, newTime, TradeRequestType::Buy);

	EXPECT_TRUE(TradeRequestDescendingComparator()(req3, req4));
	EXPECT_FALSE(TradeRequestDescendingComparator()(req4, req3));

	EXPECT_TRUE(TradeRequestAscendingComparator()(req3, req4));
	EXPECT_FALSE(TradeRequestAscendingComparator()(req4, req3));

}

TEST_F(TradeRequestTest, applyTradeTest)
{
	TradeRequest request(0, 30, 50, TradeRequestType::Buy);

	request.applyTrade(10);
	EXPECT_EQ(request.getVolume(), 20);

	request.applyTrade(20);
	EXPECT_EQ(request.getVolume(), 0);
}

TEST_F(TradeRequestTest, CreateJsonTest)
{
	using boost::posix_time::to_iso_extended_string;

	//Create request
	boost::posix_time::ptime timeOfCreation = boost::posix_time::second_clock::universal_time();
	TradeRequestType type = TradeRequestType::Buy;
	int64_t ownerID = 30;
	int64_t volume = 50;
	int64_t price = 70;

	TradeRequest request(ownerID, volume, price, timeOfCreation, type);

	nlohmann::json object;

	object["Price"] = price;
	object["Volume"] = volume;
	object["Type"] = type;
	object["UserID"] = ownerID;
	object["RegTime"] = to_iso_extended_string(timeOfCreation);

	
	nlohmann::json returnedObject = request.createJsonObject();

	EXPECT_EQ(returnedObject["Price"], object["Price"]);
	EXPECT_EQ(returnedObject["Volume"], object["Volume"]);
	EXPECT_EQ(returnedObject["Type"], object["Type"]);
	EXPECT_EQ(returnedObject["UserID"], object["UserID"]);
	EXPECT_EQ(returnedObject["RegTime"], object["RegTime"]);
}