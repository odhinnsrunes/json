#include "sdstring.hpp"

sdstring operator+(const sdstring & lhs, const std::string & rhs)
{
	sdstring ret;
	ret.reserve(lhs.size() + rhs.size());
	ret.append(lhs);
	ret.append(rhs.c_str());
	return ret;
}

sdstring operator+(const char* lhs, const sdstring & rhs)
{
	sdstring ret(lhs);
	ret.reserve(ret.size() + rhs.size());
	ret.append(rhs);
	return ret;
}

sdstring operator+(const sdstring & lhs, const char* rhs)
{
	sdstring ret(lhs);
//	ret.reserve(ret.size() + rhs.size());
	ret.append(rhs);
	return ret;
}
#if !defined USE_STD_STRING
sdstring operator+(const sdstring & lhs, const sdstring & rhs)
{
	sdstring ret;
	ret.reserve(lhs.size() + rhs.size());
	ret.append(lhs);
	ret.append(rhs);
	return ret;
}

std::string operator+(const std::string & lhs, const sdstring & rhs)
{
	std::string ret;
	ret.reserve(lhs.size() + rhs.size());
	ret.append(lhs);
	ret.append(rhs.c_str());
	return ret;
}
#endif
bool operator==(const std::string & lhs, const sdstring & rhs)
{
	return (lhs == rhs.c_str());
}

bool operator!=(const std::string & lhs, const sdstring & rhs)
{
	return (lhs != rhs.c_str());
}

