#pragma once
#include <cstdint>

inline constexpr uint16_t PORT_NUM = 45'000;

enum class RequestType : uint32_t 
{
	SignUp,
	SignIn,
	Request,
	GetInfo,
	Close
};

enum class ResponseType : uint32_t
{
	Registration,
	RequestResponse,
	ClientInfo,
	Error
};