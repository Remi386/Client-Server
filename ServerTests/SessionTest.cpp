#include "SessionTest.h"

TEST_F(SessionTest, HandleSignUpTakenLoginTest)
{
	database->registerNewUser("signUpUser1", "pass1");

	nlohmann::json message1;
	message1["Login"] = "signUpUser1";
	message1["Password"] = "somePass";

	nlohmann::json response2
		= session->handleRequest(createRequest(RequestType::SignUp, -1,
			message1));

	//Create response we expecting to get
	nlohmann::json suppMessage1;
	suppMessage1["Info"] = "This login already taken";

	checkResponse(response2, ResponseType::Registration, false,
		suppMessage1);

}

TEST_F(SessionTest, HandleSignUpCorrectTest)
{
	nlohmann::json message2;
	message2["Login"] = "SignUpUser2";
	message2["Password"] = "somePass";

	nlohmann::json response2
		= session->handleRequest(createRequest(RequestType::SignUp, -1,
											   message2));

	//We should get some id
	EXPECT_GE(response2["Message"]["UserID"], 0);

	nlohmann::json suppMessage2;

	checkResponse(response2, ResponseType::Registration, true,
				  suppMessage2);
}

TEST_F(SessionTest, HandleSignInIncorrectLoginTest)
{
	nlohmann::json message1;
	message1["Login"] = "randomLogin";
	message1["Password"] = "somePass";

	nlohmann::json response1
		= session->handleRequest(createRequest(RequestType::SignIn, -1,
			message1));

	//Create response we expecting to get
	nlohmann::json suppMessage1;
	suppMessage1["Info"] = "This login doesn't exist";

	checkResponse(response1, ResponseType::Registration, false,
		suppMessage1);
}

TEST_F(SessionTest, HandleSignInIncorrectPasswordTest)
{
	database->registerNewUser("signInUser1", "pass1");

	//Unknown password
	nlohmann::json message2;
	message2["Login"] = "signInUser1";
	message2["Password"] = "somePass";

	nlohmann::json response2
		= session->handleRequest(createRequest(RequestType::SignIn, -1,
			message2));

	//Create response we expecting to get
	nlohmann::json suppMessage2;
	suppMessage2["Info"] = "Incorrect password";

	checkResponse(response2, ResponseType::Registration, false,
		suppMessage2);
}

TEST_F(SessionTest, HandleSignInCorrectTest)
{
	int64_t userID = database->registerNewUser("signInUser2", "pass2"); 
	nlohmann::json message3;
	message3["Login"] = "signInUser2";
	message3["Password"] = "pass2";

	nlohmann::json response3
		= session->handleRequest(createRequest(RequestType::SignIn, -1,
											   message3));

	//Create response we expecting to get
	nlohmann::json suppMessage3;
	suppMessage3["UserID"] = userID;

	checkResponse(response3, ResponseType::Registration, true,
				  suppMessage3);
}

TEST_F(SessionTest, HandleTradeRequestIncorrectVolumeTest)
{

	////////////First case, incorrect volume////////////
	nlohmann::json message1;
	message1["TradeReqType"] = TradeRequestType::Buy;
	message1["Volume"] = 0;
	message1["Price"] = 40;

	nlohmann::json response1
		= session->handleRequest(createRequest(RequestType::CreateRequest, 
											   32, message1));

	nlohmann::json suppMessage1;
	suppMessage1["Info"] = "Some of arguments incorrect, request denied";

	checkResponse(response1, ResponseType::RequestResponse, false,
				  suppMessage1);

}

TEST_F(SessionTest, HandleTradeRequestIncorrectPriceTest)
{

	////////////First case, incorrect volume////////////
	///////Second case, incorrect price//////////
	nlohmann::json message2;
	message2["TradeReqType"] = TradeRequestType::Buy;
	message2["Volume"] = 34;
	message2["Price"] = 0;

	nlohmann::json response2
		= session->handleRequest(createRequest(RequestType::CreateRequest,
											   32, message2));

	nlohmann::json suppMessage2;
	suppMessage2["Info"] = "Some of arguments incorrect, request denied";

	checkResponse(response2, ResponseType::RequestResponse, false,
		suppMessage2);
}

TEST_F(SessionTest, HandleTradeRequestCorrectTest)
{
	int64_t userID = database->registerNewUser("TradeReqUser1", "pass");

	///////Third case, correct request//////////
	nlohmann::json message3;
	message3["TradeReqType"] = TradeRequestType::Buy;
	message3["Volume"] = 34;
	message3["Price"] = 45;

	nlohmann::json response3
		= session->handleRequest(createRequest(RequestType::CreateRequest,
			userID, message3));

	nlohmann::json suppMessage3;
	suppMessage3["Info"] = "Trade request successfully registered";

	checkResponse(response3, ResponseType::RequestResponse, true,
		suppMessage3);
}

TEST_F(SessionTest, HandleGetInfoRequestTest)
{
	//Register user first
	nlohmann::json regist;
	regist["Login"] = "GetInfoUser1";
	regist["Password"] = "somePass";

	nlohmann::json response
		= session->handleRequest(createRequest(RequestType::SignUp,
			45, regist));

	int64_t userID = response["Message"]["UserID"];

	//We need partner to be registered in database
	int64_t partnerID = database->registerNewUser("GetInfoRequestUser", "somePass");

	//Fill some data
	CompletedTradeRequest compReq(30, 40, partnerID,
		boost::posix_time::second_clock::universal_time(), 
		TradeRequestType::Buy);

	database->addCompletedTradeRequest(userID, compReq);

	TradeRequest actReq(userID, 45, 56, TradeRequestType::Sell);
	market->handleTradeRequest(actReq);

	ClientInfo info(46, -1200);
	database->updateClientInfo(userID, info);

	//get this data
	nlohmann::json message1;

	nlohmann::json response1
		= session->handleRequest(createRequest(RequestType::GetInfo,
			userID, message1));
	
	EXPECT_EQ(response1["ResponseType"], ResponseType::ClientInfo);
	EXPECT_EQ(response1["Status"], true);

	EXPECT_EQ(response1["Message"]["ActiveRequests"].size(), 1);
	EXPECT_EQ(response1["Message"]["TradeHistory"].size(), 1);
	
	EXPECT_EQ(response1["Message"]["Balance"][0], 46);
	EXPECT_EQ(response1["Message"]["Balance"][1], -1200);
}

TEST_F(SessionTest, HandleCancelRequestIncorrectTest)
{
	TradeRequest req(0, 0, 0, TradeRequestType::Sell);


	nlohmann::json message3 = req.createJsonObject();

	nlohmann::json response3
		= session->handleRequest(createRequest(RequestType::CancelRequest,
											   45, message3));

	nlohmann::json suppMessage3;
	suppMessage3["Info"] = "Request not found (already completed or deleted)";

	checkResponse(response3, ResponseType::CancelInfo, false,
				  suppMessage3);
}

TEST_F(SessionTest, HandleCancelRequestCorrectTest)
{
	//Register user first
	nlohmann::json regist;
	regist["Login"] = "CancelReqUser1";
	regist["Password"] = "somePass";

	nlohmann::json response
		= session->handleRequest(createRequest(RequestType::SignUp,
											   45, regist));
	
	int64_t userID = response["Message"]["UserID"];

	//Create active request
	TradeRequest req(userID, 45, 32, TradeRequestType::Sell);

	market->handleTradeRequest(req);

	nlohmann::json message = req.createJsonObject();

	//Try to cancel it
	nlohmann::json response2
		= session->handleRequest(createRequest(RequestType::CancelRequest,
											   userID, message));

	nlohmann::json suppMessage;
	suppMessage["Info"] = "Request successfully canceled";

	checkResponse(response2, ResponseType::CancelInfo, true,
				  suppMessage);
}