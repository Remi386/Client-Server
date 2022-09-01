#include <cstdint>

inline constexpr uint16_t PORT_NUM = 45'000;

enum class RequestType : uint32_t 
{
	Connection,
	Buy,
	Sell,
	Close
};