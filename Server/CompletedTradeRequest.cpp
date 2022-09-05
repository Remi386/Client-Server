#include "CompletedTradeRequest.h"

std::string CompletedTradeRequest::toString() const
{
	using boost::posix_time::to_iso_extended_string;

	return std::string((type == TradeRequestType::Buy ? "Bought " : "Sold ")
					   + std::to_string(volume) + " dollars with "
					   + std::to_string(price) + " rubles price. Published on " 
					   + to_iso_extended_string(timeOfRegistration) 
					   + ", Closed on " + to_iso_extended_string(timeOfCompletion)
					   + " PartnerID = "+ std::to_string(otherUserID));
}

nlohmann::json CompletedTradeRequest::createJsonObject() const
{
	using boost::posix_time::to_iso_extended_string;

	nlohmann::json object;

	object["Price"] = price;
	object["Volume"] = volume;
	object["Type"] = type;
	object["Partner"] = otherUserID;
	object["RegTime"] = to_iso_extended_string(timeOfRegistration);
	object["CloseTime"] = to_iso_extended_string(timeOfCompletion);

	return object;
}