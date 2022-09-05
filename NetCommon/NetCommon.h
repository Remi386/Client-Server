#pragma once
#include <cstdint>

inline constexpr uint16_t PORT_NUM = 45'000;

enum class RequestType : uint32_t 
{
	SignUp,
	SignIn,
	CreateRequest,
	GetInfo,
	GetQuotes,
	CancelRequest,
	Close
};

enum class ResponseType : uint32_t
{
	Registration,
	RequestResponse,
	ClientInfo,
	CancelInfo,
	QuotesInfo,
	RequestCompleted,
	Error
};

enum class TradeRequestType : uint8_t
{
	Buy,
	Sell
};