#include "CompletedTradeRequestTest.h"

TEST_F(CompletedTradeRequestTest, CostructorTest)
{
	CompletedTradeRequest req1(1, 2, 3, timeOfCreation, TradeRequestType::Buy);

	EXPECT_EQ(req1.getVolume(), 1);
	EXPECT_EQ(req1.getPrice(), 2);
	EXPECT_EQ(req1.getOtherUserID(), 3);
	EXPECT_EQ(req1.getRegistrationTime(), timeOfCreation);
	EXPECT_EQ(req1.getType(), TradeRequestType::Buy);

	CompletedTradeRequest req2(4000, 11, 15, timeOfCreation, TradeRequestType::Sell);

	EXPECT_EQ(req2.getVolume(), 4000);
	EXPECT_EQ(req2.getPrice(), 11);
	EXPECT_EQ(req2.getOtherUserID(), 15);
	EXPECT_EQ(req2.getType(), TradeRequestType::Sell);
}

TEST_F(CompletedTradeRequestTest, CreateJsonTest)
{
	using boost::posix_time::to_iso_extended_string;

	int64_t price = 10;
	int64_t volume = 20;
	int64_t otherUserID = 50;
	TradeRequestType type = TradeRequestType::Buy;

	nlohmann::json object;

	object["Price"] = price;
	object["Volume"] = volume;
	object["Type"] = type;
	object["Partner"] = otherUserID;
	object["RegTime"] = to_iso_extended_string(timeOfCreation);

	CompletedTradeRequest req(volume, price, otherUserID, timeOfCreation, type);

	object["CloseTime"] = to_iso_extended_string(req.getComletionTime());

	nlohmann::json returnedObject = req.createJsonObject();

	EXPECT_EQ(returnedObject["Price"], object["Price"]);
	EXPECT_EQ(returnedObject["Volume"], object["Volume"]);
	EXPECT_EQ(returnedObject["Type"], object["Type"]);
	EXPECT_EQ(returnedObject["Partner"], object["Partner"]);
	EXPECT_EQ(returnedObject["RegTime"], object["RegTime"]);
	EXPECT_EQ(returnedObject["CloseTime"], object["CloseTime"]);
}