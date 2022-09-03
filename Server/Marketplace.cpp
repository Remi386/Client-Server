#include "Marketplace.h"
#include "DataBase.h"

void Marketplace::handleTradeRequest(TradeRequest& tradeRequest_)
{
	using TradeType = TradeRequest::Type;

	auto RequestHandler = [this](auto& requestContainer,  //Container simular to request, e.i. for buy request this is buyRequests container 
								 auto& oppositeContainer, //Container opposite to request, e.i. for buy request this is sellRequests container 
								 TradeRequest& tradeRequest, 
								 TradeType firstRequestType,
								 TradeType secondRequestType)
	{ // lambda body
		int64_t firstUserID = tradeRequest.getOwner();

		if (oppositeContainer.empty()) {
			addRequest(tradeRequest, firstRequestType);
		}
		else {
			auto iter = oppositeContainer.begin();
			auto end = oppositeContainer.end();

			while (iter != end && tradeRequest.getVolume() > 0)
			{
				int64_t secondUserID = iter->getOwner();

				//tradeVolume - dollars to buy/sell
				int64_t tradeVolume = std::min(iter->getVolume(), tradeRequest.getVolume());
				int64_t rublesVolume = tradeVolume * iter->getPrice();

				//Register trade between users
				CompletedTradeRequest firstTrade(tradeVolume, iter->getPrice(),
											     secondUserID, firstRequestType);

				CompletedTradeRequest secondTrade(tradeVolume, iter->getPrice(),
												  firstUserID, secondRequestType);

				auto& db = DataBase::instance();
				//Add trade to history
				db.AddCompletedTradeRequest(firstUserID, firstTrade);
				db.AddCompletedTradeRequest(secondUserID, secondTrade);

				//change users balance
				db.getClientInfo(firstUserID).applyBalanceChanges(tradeVolume, rublesVolume,
																  firstRequestType);

				db.getClientInfo(secondUserID).applyBalanceChanges(tradeVolume, rublesVolume,
																   secondRequestType);

				//Apply chages to trade request for further condition checks
				iter->applyTrade(tradeVolume);
				tradeRequest.applyTrade(tradeVolume);

				//Remove request from active if it is satisfied
				if (iter->isCompleted()) {
					//send iterator copy to erase, then increment it
					removeRequest(*(iter++), secondRequestType);
				}
				else {
					++iter;
				}
			} //while

			//If main request is not satisfied - add it to active
			if (!tradeRequest.isCompleted()) {
				addRequest(tradeRequest, firstRequestType);
			}
		} //else
	};

	switch (tradeRequest_.getType())
	{
	case TradeRequest::Type::Buy:
		RequestHandler(buyRequests, sellRequests, tradeRequest_, TradeType::Buy, TradeType::Sell);
		break;
	case TradeRequest::Type::Sell:
		RequestHandler(sellRequests, buyRequests, tradeRequest_, TradeType::Sell, TradeType::Buy);
		break;
	}
}

void Marketplace::addRequest(TradeRequest& req, TradeRequest::Type reqType)
{
	const TradeRequest* insertedValue;

	switch (reqType)
	{
	case TradeRequest::Type::Buy:
		insertedValue = &(*(buyRequests.insert(req)));
		break;
	case TradeRequest::Type::Sell:
		insertedValue = &(*(sellRequests.insert(req)));
		break;
	}

	activeRequests[req.getOwner()].push_back(insertedValue);
}

void Marketplace::removeRequest(const TradeRequest& req, TradeRequest::Type reqType)
{

	listOfRequests& clientRequests = activeRequests.at(req.getOwner());
	auto iter = std::find(clientRequests.begin(), clientRequests.end(), &req);
	
	if (iter != clientRequests.end()) {
		clientRequests.erase(iter);
	}

	if (clientRequests.empty()) { //If client no longer have request
		activeRequests.erase(req.getOwner());
	}

	switch (reqType)
	{
	case TradeRequest::Type::Buy:
		buyRequests.erase(req);
		break;
	case TradeRequest::Type::Sell:
		sellRequests.erase(req);
		break;
	}
}

const Marketplace::listOfRequests& 
	Marketplace::getActiveRequests(int64_t userID)
{
	auto iter = activeRequests.find(userID);

	if (iter != activeRequests.end())
		return activeRequests[userID];
	else {
		static const listOfRequests empty; //As long as we return a reference, we should 
										   //care about object lifetime
		return empty;
	}
}

/*
if (buyRequests.empty()) {
			sellRequests.insert(tradeRequest);
		}
		else {
			auto iter = buyRequests.begin();
			auto end = buyRequests.end();
			int64_t firstUserID = tradeRequest.getOwner();

			while (iter != end || tradeRequest.getVolume() > 0)
			{
				int64_t secondUserID = iter->getOwner();

				int64_t tradeVolume = std::min(iter->getVolume(), tradeRequest.getVolume());
				int64_t rublesVolume = tradeVolume * iter->getPrice();

				CompletedTradeRequest firstTrade(tradeVolume, iter->getPrice(),
					secondUserID, TradeRequest::Type::Buy);

				CompletedTradeRequest secondTrade(tradeVolume, iter->getPrice(),
					firstUserID, TradeRequest::Type::Sell);

				auto db = DataBase::instance();
				db.AddCompletedTradeRequest(firstUserID, firstTrade);
				db.AddCompletedTradeRequest(secondUserID, secondTrade);

				db.getClientInfo(firstUserID).applyBalanceChanges(tradeVolume, rublesVolume,
					TradeRequest::Type::Buy);

				db.getClientInfo(secondUserID).applyBalanceChanges(tradeVolume, rublesVolume,
					TradeRequest::Type::Sell);


				iter->isCompleted() ? sellRequests.erase(iter++) : ++iter;
			}

			if (tradeRequest.getVolume() != 0) {
				buyRequests.insert(tradeRequest);
			}
		}
*/