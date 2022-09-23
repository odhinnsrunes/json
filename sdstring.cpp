/*
Copyright (c) 2012-2022 James Baker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

The official repository for this library is at https://github.com/odhinnsrunes/json

*/
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

