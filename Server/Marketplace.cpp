#include "Marketplace.h"
#include "DataBase.h"
#include "Session.h"

void Marketplace::handleTradeRequest(TradeRequest& tradeRequest_)
{
	auto RequestHandler = [this](auto& requestContainer,  //Container simular to request, e.i. for buy request this is buyRequests container 
								 auto& oppositeContainer, //Container opposite to request, e.i. for buy request this is sellRequests container 
								 TradeRequest& tradeRequest, 
								 TradeRequestType firstRequestType,
								 TradeRequestType secondRequestType)
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

				auto& db = DataBase::instance();

				//If two requests not from one user
				if (firstUserID != secondUserID) { 

					//Register trade between users
					CompletedTradeRequest firstTrade(tradeVolume, iter->getPrice(),
													 secondUserID, 
													 tradeRequest.getRegistrationTime(), 
													 firstRequestType);

					CompletedTradeRequest secondTrade(tradeVolume, iter->getPrice(),
												      firstUserID, 
													  iter->getRegistrationTime(),
													  secondRequestType);


					std::weak_ptr<Session> firstSession = getSession(firstUserID);
					std::weak_ptr<Session> secondSession = getSession(secondUserID);

					//Send notification to first user
					if (auto fSession = firstSession.lock()) {
						fSession->sendNotification(firstTrade.toString());
					}

					//Send notification to second user
					if (auto sSession = secondSession.lock()) {
						sSession->sendNotification(secondTrade.toString());
					}

					//Add trade to history
					db.addCompletedTradeRequest(firstUserID, firstTrade);
					db.addCompletedTradeRequest(secondUserID, secondTrade);

					//change users balance
					db.getClientInfo(firstUserID).applyBalanceChanges(tradeVolume, rublesVolume,
						firstRequestType);

					db.getClientInfo(secondUserID).applyBalanceChanges(tradeVolume, rublesVolume,
						secondRequestType);

				}
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
	case TradeRequestType::Buy:
		RequestHandler(buyRequests, sellRequests, tradeRequest_, 
					   TradeRequestType::Buy, TradeRequestType::Sell);
		break;
	case TradeRequestType::Sell:
		RequestHandler(sellRequests, buyRequests, tradeRequest_, 
					   TradeRequestType::Sell, TradeRequestType::Buy);
		break;
	}
}

void Marketplace::addRequest(TradeRequest& req, TradeRequestType reqType)
{
	const TradeRequest* insertedValue;

	switch (reqType)
	{
	case TradeRequestType::Buy:
		insertedValue = &(*(buyRequests.insert(req)));
		break;
	case TradeRequestType::Sell:
		insertedValue = &(*(sellRequests.insert(req)));
		break;
	}

	activeRequests[req.getOwner()].push_back(insertedValue);
}

void Marketplace::removeRequest(const TradeRequest& req, TradeRequestType reqType)
{

	listOfRequests& clientRequests = activeRequests.at(req.getOwner());
	auto iter = std::find(clientRequests.begin(), clientRequests.end(), &req);

	if (iter != clientRequests.end()) {
		clientRequests.erase(iter);
	}

	if (clientRequests.empty()) { //If client no longer have requests
		activeRequests.erase(req.getOwner());
	}

	switch (reqType)
	{
	case TradeRequestType::Buy:
	{
		auto iter = buyRequests.find(req);

		if (iter != buyRequests.end()) {
			buyRequests.erase(iter);
			break;
		}
	}
	case TradeRequestType::Sell:
	{
		auto iter = sellRequests.find(req);

		if (iter != sellRequests.end()) {
			sellRequests.erase(iter);
			break;
		}
	}
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

bool Marketplace::cancelTradeRequest(const TradeRequest& req)
{
	auto client = activeRequests.find(req.getOwner());

	if (client == activeRequests.end())
		return false;

	listOfRequests& clientRequests = client->second;

	auto iter = std::find_if(clientRequests.begin(), clientRequests.end(),
		[&](const TradeRequest* clientRequest) 
		{
			return req.getPrice() == clientRequest->getPrice()
				&& req.getRegistrationTime() == clientRequest->getRegistrationTime();
		});

	if (iter == clientRequests.end()) {
		return false;
	}

	clientRequests.erase(iter);

	if (clientRequests.empty()) { //If client no longer have requests
		activeRequests.erase(req.getOwner());
	}

	switch (req.getType())
	{
	case TradeRequestType::Buy:
	{
		auto iter = buyRequests.find(req);

		if (iter != buyRequests.end()) {
			buyRequests.erase(iter);
			break;
		}
		return false;
	}
	case TradeRequestType::Sell:
	{
		auto iter = sellRequests.find(req);

		if (iter != sellRequests.end()) {
			sellRequests.erase(iter);
			break;
		}
		return false;
	}
	}

	return true;
}
