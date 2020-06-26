/*
Copyright (c) 2012-2020 James Baker

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

#include <assert.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <stdarg.h>
#if defined _WINDOWS && defined __clang__
#define __uncaught_exception std::uncaught_exception
#endif
// #include <thread>
#include <sys/types.h>
#include <sys/stat.h>

#if defined _USE_ADDED_ORDER_
#undef _USE_ADDED_ORDER_
#if !defined SUPPORT_ORDERED_JSON
#define SUPPORT_ORDERED_JSON
#endif
#include "json.hpp"
#define _USE_ADDED_ORDER_
// #undef JSON_HPP_
// #include "json.hpp"
#define JSON_NAMESPACE ojson
#else
#include "json.hpp"
#define JSON_NAMESPACE json
#endif

#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn - 8026
#pragma warn - 8027
#endif

#if defined __GNUC__ || __BORLANDC__ >= 0x0600
#define UNSIGNEDMAX 0xFFFFFFFFFFFFFFFFLL
#define SIGNEDMAX 0x7FFFFFFFFFFFFFFFLL
#else
#define UNSIGNEDMAX 0xFFFFFFFFFFFFFFFF
#define SIGNEDMAX 0x7FFFFFFFFFFFFFFF
#endif

#if defined _WINDOWS
#define _CRT_SECURE_NO_DEPRICATE 1
#define _CRT_SECURE_NO_WARNINGS 1
#endif

namespace JSON_NAMESPACE
{
	void debug(const char * format, ...) {
		sdstring s;
		size_t size = 100;
		bool b = false;
		va_list marker;
		while (!b) {
			size_t n;
			s.resize(size);
			va_start(marker, format);
			n = vsnprintf((char*)s.c_str(), size, format, marker);
			va_end(marker);
			b = (n < size);
			if (n > 0 && n != (size_t)-1 && b) {
				size = n;
			} else if (n == (size_t)-1) {
				size = size * 2;
			} else {
				size = n * 2;
			}
		}
		printf("%s\n", s.c_str());
	}

	class MovingCharPointer
	{
	public:
		MovingCharPointer(sdstring & in, size_t reserve);
		MovingCharPointer(MovingCharPointer&& ptr);
		~MovingCharPointer() {}

		MovingCharPointer& operator=(MovingCharPointer&& ptr);

		void set(const sdstring& str);
		void set(const char* n, size_t size);
		void set(char n);

		char* orig();

		char* move();
	private:
		sdstring & loc;
		char* m_orig;
		char* m_current;
		char* m_max;
	};

	const double e[] = { // 1e-308...1e308: 617 * 8 bytes = 4936 bytes
		1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301, 1e-300,
		1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291, 1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282, 1e-281, 1e-280,
		1e-279, 1e-278, 1e-277, 1e-276, 1e-275, 1e-274, 1e-273, 1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264, 1e-263, 1e-262, 1e-261, 1e-260,
		1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254, 1e-253, 1e-252, 1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246, 1e-245, 1e-244, 1e-243, 1e-242, 1e-241, 1e-240,
		1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228, 1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221, 1e-220,
		1e-219, 1e-218, 1e-217, 1e-216, 1e-215, 1e-214, 1e-213, 1e-212, 1e-211, 1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204, 1e-203, 1e-202, 1e-201, 1e-200,
		1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192, 1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183, 1e-182, 1e-181, 1e-180,
		1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174, 1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165, 1e-164, 1e-163, 1e-162, 1e-161, 1e-160,
		1e-159, 1e-158, 1e-157, 1e-156, 1e-155, 1e-154, 1e-153, 1e-152, 1e-151, 1e-150, 1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144, 1e-143, 1e-142, 1e-141, 1e-140,
		1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132, 1e-131, 1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121, 1e-120,
		1e-119, 1e-118, 1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111, 1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102, 1e-101, 1e-100,
		1e-99, 1e-98, 1e-97, 1e-96, 1e-95, 1e-94, 1e-93, 1e-92, 1e-91, 1e-90, 1e-89, 1e-88, 1e-87, 1e-86, 1e-85, 1e-84, 1e-83, 1e-82, 1e-81, 1e-80,
		1e-79, 1e-78, 1e-77, 1e-76, 1e-75, 1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69, 1e-68, 1e-67, 1e-66, 1e-65, 1e-64, 1e-63, 1e-62, 1e-61, 1e-60,
		1e-59, 1e-58, 1e-57, 1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49, 1e-48, 1e-47, 1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41, 1e-40,
		1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21, 1e-20,
		1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e+0,
		1e+1, 1e+2, 1e+3, 1e+4, 1e+5, 1e+6, 1e+7, 1e+8, 1e+9, 1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15, 1e+16, 1e+17, 1e+18, 1e+19, 1e+20,
		1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31, 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40,
		1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47, 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60,
		1e+61, 1e+62, 1e+63, 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79, 1e+80,
		1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95, 1e+96, 1e+97, 1e+98, 1e+99, 1e+100,
		1e+101, 1e+102, 1e+103, 1e+104, 1e+105, 1e+106, 1e+107, 1e+108, 1e+109, 1e+110, 1e+111, 1e+112, 1e+113, 1e+114, 1e+115, 1e+116, 1e+117, 1e+118, 1e+119, 1e+120,
		1e+121, 1e+122, 1e+123, 1e+124, 1e+125, 1e+126, 1e+127, 1e+128, 1e+129, 1e+130, 1e+131, 1e+132, 1e+133, 1e+134, 1e+135, 1e+136, 1e+137, 1e+138, 1e+139, 1e+140,
		1e+141, 1e+142, 1e+143, 1e+144, 1e+145, 1e+146, 1e+147, 1e+148, 1e+149, 1e+150, 1e+151, 1e+152, 1e+153, 1e+154, 1e+155, 1e+156, 1e+157, 1e+158, 1e+159, 1e+160,
		1e+161, 1e+162, 1e+163, 1e+164, 1e+165, 1e+166, 1e+167, 1e+168, 1e+169, 1e+170, 1e+171, 1e+172, 1e+173, 1e+174, 1e+175, 1e+176, 1e+177, 1e+178, 1e+179, 1e+180,
		1e+181, 1e+182, 1e+183, 1e+184, 1e+185, 1e+186, 1e+187, 1e+188, 1e+189, 1e+190, 1e+191, 1e+192, 1e+193, 1e+194, 1e+195, 1e+196, 1e+197, 1e+198, 1e+199, 1e+200,
		1e+201, 1e+202, 1e+203, 1e+204, 1e+205, 1e+206, 1e+207, 1e+208, 1e+209, 1e+210, 1e+211, 1e+212, 1e+213, 1e+214, 1e+215, 1e+216, 1e+217, 1e+218, 1e+219, 1e+220,
		1e+221, 1e+222, 1e+223, 1e+224, 1e+225, 1e+226, 1e+227, 1e+228, 1e+229, 1e+230, 1e+231, 1e+232, 1e+233, 1e+234, 1e+235, 1e+236, 1e+237, 1e+238, 1e+239, 1e+240,
		1e+241, 1e+242, 1e+243, 1e+244, 1e+245, 1e+246, 1e+247, 1e+248, 1e+249, 1e+250, 1e+251, 1e+252, 1e+253, 1e+254, 1e+255, 1e+256, 1e+257, 1e+258, 1e+259, 1e+260,
		1e+261, 1e+262, 1e+263, 1e+264, 1e+265, 1e+266, 1e+267, 1e+268, 1e+269, 1e+270, 1e+271, 1e+272, 1e+273, 1e+274, 1e+275, 1e+276, 1e+277, 1e+278, 1e+279, 1e+280,
		1e+281, 1e+282, 1e+283, 1e+284, 1e+285, 1e+286, 1e+287, 1e+288, 1e+289, 1e+290, 1e+291, 1e+292, 1e+293, 1e+294, 1e+295, 1e+296, 1e+297, 1e+298, 1e+299, 1e+300,
		1e+301, 1e+302, 1e+303, 1e+304, 1e+305, 1e+306, 1e+307, 1e+308};

	template <typename CharType = char>
	struct UTF8
	{
		typedef CharType Ch;

		static Ch* Encode(Ch* buffer, unsigned codepoint) {
			if (codepoint <= 0x7F)
				*buffer++ = codepoint & 0xFF;
			else if (codepoint <= 0x7FF) {
				*buffer++ = 0xC0 | ((codepoint >> 6) & 0xFF);
				*buffer++ = 0x80 | ((codepoint & 0x3F));
			} else if (codepoint <= 0xFFFF) {
				*buffer++ = 0xE0 | ((codepoint >> 12) & 0xFF);
				*buffer++ = 0x80 | ((codepoint >> 6) & 0x3F);
				*buffer++ = 0x80 | (codepoint & 0x3F);
			} else {
				if (!(codepoint <= 0x10FFFF))
					return buffer;
				*buffer++ = 0xF0 | ((codepoint >> 18) & 0xFF);
				*buffer++ = 0x80 | ((codepoint >> 12) & 0x3F);
				*buffer++ = 0x80 | ((codepoint >> 6) & 0x3F);
				*buffer++ = 0x80 | (codepoint & 0x3F);
			}
			return buffer;
		}
	};

	double Pow10(int n) {
		if (!(n <= 308))
			return 0.0;
		return n < -308 ? 0.0 : e[n + 308];
	}

	void generateError(instring& inputString, const char* szError) {
		sdstring in = inputString.SoFar();
		size_t l = in.size();
		size_t pos = 1;
		size_t line = 1;
		for (size_t i = 0; i < l; i++) {
			if (in[i] == '\n') {
				line++;
				pos = 1;
			} else {
				if (in[i] != '\r')
					pos++;
			}
		}

		std::ostringstream s;
		s << szError << "  Line: " << line <<  " Column: " << pos;
		instring ostring(s.str().c_str());
		inputString = ostring;
	}

	void nullParse(value& ret, instring& inputString, bool* bFailed) {
		if (inputString.take() != 'n' || inputString.take() != 'u' || inputString.take() != 'l' || inputString.take() != 'l') {
			generateError(inputString, "Error Parsing null.");
			*bFailed = true;
		}
		if (ret.myType == JSON_OBJECT)
			delete ret.obj;
		else if (ret.myType == JSON_ARRAY)
			delete ret.arr;
		ret.myType = JSON_NULL;

		ret.str.clear();

		ret.m_number = 0.0;
		ret.m_places = -1;

		ret.obj = NULL;
		ret.arr = NULL;
		ret.m_boolean = false;
	}

	void trueParse(value& ret, instring& inputString, bool* bFailed) {
		if (inputString.take() != 't' || inputString.take() != 'r' || inputString.take() != 'u' || inputString.take() != 'e') {
			generateError(inputString, "Error Parsing true.");
			*bFailed = true;
		}
		ret = true;
	}

	void falseParse(value& ret, instring& inputString, bool* bFailed) {
		if (inputString.take() != 'f' || inputString.take() != 'a' || inputString.take() != 'l' || inputString.take() != 's' || inputString.take() != 'e') {
			generateError(inputString, "Error Parsing false.");
			*bFailed = true;
		}
		ret = false;
	}

	void SkipWhitespace(instring& in) {
		while (in.peek() == ' ' || in.peek() == '\n' || in.peek() == '\r' || in.peek() == '\t')
			in.skip();
	}

	unsigned hex4Parse(instring& s, bool* bFailed) {
		unsigned ret = 0;
		for (int i = 0; i < 4; i++) {
			char &c = s.take();
			ret <<= 4;
			ret += (unsigned)c;
			if (c >= '0' && c <= '9')
				ret -= '0';
			else if (c >= 'A' && c <= 'F')
				ret -= 'A' - 10;
			else if (c >= 'a' && c <= 'f')
				ret -= 'a' - 10;
			else {
				generateError(s, "Error Parsing hex.");
				*bFailed = true;
				return 0;
			}
		}
		return ret;
	}

	void stringParse(sdstring& ret, instring& s, bool* bFailed) {
#define Z16 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		static const char escape[256] = {
			Z16, Z16, 0, 0, '\"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '/',
			Z16, Z16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,
			0, 0, '\b', 0, 0, 0, '\f', 0, 0, 0, 0, 0, 0, 0, '\n', 0,
			0, 0, '\r', 0, '\t', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16};
#undef Z16

		s.take(); // Skip '\"'

		char* ptr = s.getPos();
		char* retVal = ptr;
		for (;;) {
			char &c = s.take();
			switch (c) {
				default: {
					*ptr = c;
					++ptr;
					// *ptr++ = c;
					break;
				}

				case '"': {
					ret.assign(retVal, (size_t)(ptr - retVal));
					return;
				}

				case '\\': {
					char &cE = s.take();
					if (escape[(unsigned char)cE])
						*ptr++ = escape[(unsigned char)cE];
					else if (cE == 'u') { // Unicode
						unsigned h = hex4Parse(s, bFailed);
						if (h >= 0xD800 && h <= 0xDBFF) { // Handle UTF-16 surrogate pair
							if (s.take() != '\\' || s.take() != 'u') {
								generateError(s, "Error Parsing string.");
								*bFailed = true;
								return;
							}
							unsigned h2 = hex4Parse(s, bFailed);
							if (h2 < 0xDC00 || h2 > 0xDFFF) {
								generateError(s, "Error Parsing string.");
								*bFailed = true;
								return;
							}
							h = (((h - 0xD800) << 10) | (h2 - 0xDC00)) + 0x10000;
						}

						char buffer[4];
						size_t count = size_t(UTF8<>::Encode(buffer, h) - &buffer[0]);
						for (size_t i = 0; i < count; i++) {
							*ptr++ = buffer[i];
						}
					} else {
						generateError(s, "Unknown escape character.");
						*bFailed = true;
						return;
					}

					break;
				}

				case 0: {
					generateError(s, "lacks ending quotation before the end of string");
					*bFailed = true;
					return;
				}
			}
		}
	}

	void numberParse(value& ret, instring& s, bool* bFailed) {
		char * pStart = s.getPos();
		bool minus = false;
		if (s.peek() == '-') {
			minus = true;
			s.take();
		}

		double d = 0;

		if (s.peek() >= '0' && s.peek() <= '9') {
			d = s.take() - '0';

			while (s.peek() >= '0' && s.peek() <= '9') {
				if (d >= 1E307) {
					generateError(s, "Number too big to store in double");
					*bFailed = true;
					ret = value();
					return;
				}
				d = d * 10 + (s.take() - '0');
			}
		} else if (s.peek() != '.') {
			generateError(s, "Expect a value here.");
			*bFailed = true;
			ret = value();
			return;
		}

		int expFrac = 0;
		if (s.peek() == '.') {
			s.take();

			if (s.peek() >= '0' && s.peek() <= '9') {
				d = d * 10 + (s.take() - '0');
				--expFrac;
			} else {
				generateError(s, "At least one digit in fraction part");
				*bFailed = true;
				ret = value();
				return; // value();
			}

			while (s.peek() >= '0' && s.peek() <= '9') {
				if (expFrac > -16) {
					d = d * 10 + (s.peek() - '0');
					--expFrac;
				}
				s.take();
			}
		}

		int exp = 0;
		if (s.peek() == 'e' || s.peek() == 'E') {
			s.take();

			bool expMinus = false;
			if (s.peek() == '+')
				s.take();
			else if (s.peek() == '-') {
				s.take();
				expMinus = true;
			}

			if (s.peek() >= '0' && s.peek() <= '9') {
				exp = s.take() - '0';
				while (s.peek() >= '0' && s.peek() <= '9') {
					exp = exp * 10 + (s.take() - '0');
					if (exp > 308) {
						generateError(s, "Number too big to store in double");
						*bFailed = true;
						ret = value();
						return;
					}
				}
			} else {
				generateError(s, "At least one digit in exponent");
				*bFailed = true;
				ret = value();
				return;
			}

			if (expMinus)
				exp = -exp;
		}

		d *= Pow10(exp + expFrac);
		ret = minus ? -d : d;
		ret.str.assign(pStart, (size_t)(s.getPos() - pStart));
		size_t pos = 0;
		for (char c : ret.str) {
			if (c != '0') {
				if (pos > 0) {
					ret.str = ret.str.substr(pos);
				}
				if (ret.str[0] == '.') {
					ret.str.insert(0, 1, '0');
				}
				break;
			}
			pos++;
		}
		return;
	}

	void objectParse(value& ret, instring& inputString, bool* bFailed) {
		if (inputString.peek() != '{') {
			generateError(inputString, "Invalid character for start of object.");
			*bFailed = true;
			return;
		}
		inputString.take();

		ret.myType = JSON_OBJECT;
		SkipWhitespace(inputString);
//		if (ret.obj == NULL) {
			ret.obj = new object();
//			if (ret.arr)
//				delete ret.arr;
//			ret.arr = NULL;

			ret.obj->setNotEmpty();
			if (ret.pParentArray) {
				ret.obj->setParentArray(ret.pParentArray);
			} else if (ret.pParentObject) {
				ret.obj->setParentObject(ret.pParentObject);
			}
//		}
		if (inputString.peek() == '}') {
			inputString.take();
			return;
		}

		
		for (;;) {
			if (inputString.peek() != '"') {
				generateError(inputString, "Name of an object member must be a string");
				*bFailed = true;
				ret = value();
				return;
			}
			sdstring key;
			stringParse(key, inputString, bFailed);
			if (*bFailed) {
				ret = value();
				return;
			}

			SkipWhitespace(inputString);

			if (inputString.take() != ':') {
				generateError(inputString, "There must be a colon after the name of object member");
				*bFailed = true;
				ret = value();
				return;
			}
			SkipWhitespace(inputString);
			value &temp = (*ret.obj)[key];
			temp.setParentObject(ret.obj);
			valueParse(temp, inputString, bFailed);
			if (*bFailed) {
				ret = value();
				return;
			}

			SkipWhitespace(inputString);

			switch (inputString.take()) {
			case ',':
				SkipWhitespace(inputString);
				break;

			case '}':
				return;
			default:
				generateError(inputString, "Must be a comma or '}' after an object member");
				*bFailed = true;
				ret = value();
				return;
			}
		}
	}

	void arrayParse(value& arr, instring& inputString, bool* bFailed) {
		if (inputString.peek() != '[') {
			generateError(inputString, "Invalid character for start of object.");
			*bFailed = true;
			arr = value();
			return;
		}
		inputString.take();

		arr.myType = JSON_ARRAY;
		SkipWhitespace(inputString);
//		if (arr.arr == NULL) {
			arr.arr = new array();
//			if (arr.obj)
//				delete arr.obj;
//			arr.obj = NULL;
			if (arr.pParentArray) {
				arr.arr->setParentArray(arr.pParentArray);
			} else if (arr.pParentObject) {
				arr.arr->setParentObject(arr.pParentObject);
			}

			arr.arr->setNotEmpty();
//		}

		if (inputString.peek() == ']') {
			inputString.take();
			return;
		}

		for (;;) {

			size_t l = arr.arr->size();
			arr.arr->resize(l + 1);
			value & temp = arr.arr->at(l);
			temp.setParentArray(arr.arr);
			valueParse(temp, inputString, bFailed);
			if (*bFailed) {
				arr = value();
				return;
			}

			SkipWhitespace(inputString);
			char c = inputString.take();
			switch (c) {
			case ',':
				SkipWhitespace(inputString);
				break;

			case ']':
				return;
			default:
				generateError(inputString, "Must be a comma or ']' after an array element.");
				*bFailed = true;
				arr = value();
				return;
			}
		}
	}

	void valueParse(value& a, instring& inputString, bool* bFailed)
	{
		switch (inputString.peek()) {
		case 'n':
			nullParse(a, inputString, bFailed);
			return;
		case 't':
			trueParse(a, inputString, bFailed);
			return;
		case 'f':
			falseParse(a, inputString, bFailed);
			return;
		case '"': {
			if (a.myType == JSON_OBJECT)
				delete a.obj;
			if (a.myType == JSON_ARRAY)
				delete a.arr;
			a.myType = JSON_STRING;
			a.m_boolean = false;
			a.m_number = 0.0;
			a.m_places = -1;
			if (!a.str.empty())
				a.str.clear();

			stringParse(a.str, inputString, bFailed);
			return;
		}
		case '{':
			objectParse(a, inputString, bFailed);
			return;
		case '[':
			arrayParse(a, inputString, bFailed);
			return;
		}
		numberParse(a, inputString, bFailed);
	}

	size_t esize(const sdstring& ins) {
		size_t ret = 0;
		static const char escape[256] = {
		//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
			6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 6, 2, 2, 6, 6, // 00
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 10
			1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 20
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 30
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	// 40
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,	// 50
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 60
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 70
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 80
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 90
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // A0
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // B0
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // C0
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // D0
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // E0
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1	// F0
		};
		size_t l = ins.size();
		const char * in = ins.c_str();
		for (size_t i = 0; i < l; i++) {
			ret += (size_t)escape[(unsigned char)(*(in++))];
		}
		return ret;
	}

	void escape(MovingCharPointer& ptr, const sdstring& ins)
	{
		static const char hexDigits[] = "0123456789ABCDEF";
		static const char escape[256] = {
		//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
			 0,   0,  '"',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 20
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 30
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 40																// 30~4F
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  '\\', 0,   0,   0,  // 50
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 60
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 70
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 80
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 90
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // A0
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // B0
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // C0
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // D0
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // E0
			 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // F0
		};

		size_t l = ins.size();
		const char* in = ins.c_str();
		for (size_t i = 0; i < l; i++, in++) {
			switch (escape[(unsigned char)(*in)]) {
			case 0:
				ptr.set((*in));
				break;
			case 'u':
				ptr.set("\\u00", 4);
				ptr.set(hexDigits[(*in) >> 4]);
				ptr.set(hexDigits[(*in) & 0xF]);
				break;
			default:
				ptr.set('\\');
				ptr.set(escape[(unsigned char)(*in)]);
				break;
			}
		}
	}

	MovingCharPointer::MovingCharPointer(sdstring &in, size_t reserve) :
		loc(in)
	{
		loc.assign(reserve, 0);
		m_orig = (char*)loc.data();
		m_current = m_orig;
		m_max = m_orig + reserve;
//		m_orig[reserve] = 0;
//		m_current = m_orig;
//		m_max = m_orig + reserve;
	}


	MovingCharPointer::MovingCharPointer(MovingCharPointer&& ptr) :
		loc(ptr.loc),
		m_orig(nullptr),
		m_current(nullptr),
		m_max(nullptr)
	{
		std::swap(m_orig, ptr.m_orig);
		std::swap(m_current, ptr.m_current);
		std::swap(m_max, ptr.m_max);
	}

	MovingCharPointer& MovingCharPointer::operator=(MovingCharPointer&& ptr)
	{
		loc = ptr.loc;
		std::swap(m_orig, ptr.m_orig);
		std::swap(m_current, ptr.m_current);
		std::swap(m_max, ptr.m_max);
		return *this;
	}

	inline void MovingCharPointer::set(const sdstring& str) {
		size_t l = str.size();
		memcpy(m_current, str.c_str(), l);
		m_current += l;
	}

	inline void MovingCharPointer::set(const char* n, size_t size) {
		memcpy(m_current, n, size);
		m_current += size;
	}

	inline void MovingCharPointer::set(char n) {
		*m_current = n;
		++m_current;
	}

	char* MovingCharPointer::orig() {
		*m_current = 0;
		return m_orig;
	}

	char* MovingCharPointer::move() {
		*m_current = 0;
		char* temp = m_orig;
		m_orig = nullptr;
		m_current = nullptr;
		m_max = nullptr;
		return temp;
	}

	instring::instring(const sdstring& in) {
		m_size = in.size();
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
	}

	instring::instring(const instring& in) {
			m_size = in.m_size;
		str = static_cast<char*>(malloc(m_size + 1));
			wpos = str + (in.wpos - in.str);
		memcpy(str, in.str, m_size);
		str[m_size] = 0;
	}

	instring::instring(char* in) {
		m_size = strlen(in);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in, m_size);
		str[m_size] = 0;
	}

	instring::~instring() {
		memset(str, 0, m_size);
		free(str);
	}

	void instring::seek(size_t newPos) {
		if (newPos < m_size) {
			wpos = str + newPos;
		}
	}

	char* instring::getPos() {
		return wpos;
	}

	instring& instring::operator=(const sdstring& in) {
		memset(str, 0, m_size);
		m_size = in.size();
		free(str);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
		return *this;
	}

	instring& instring::operator=(const char* in) {
		memset(str, 0, m_size);
		m_size = strlen(in);
		free(str);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in, m_size);
		str[m_size] = 0;
		return *this;
	}

	instring& instring::operator=(const instring& in) {
		if (this == &in)
			return *this;
		memset(str, 0, m_size);
		m_size = in.m_size;
		free(str);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str + (in.wpos - in.str);
		memcpy(str, in.str, m_size);
		str[m_size] = 0;
		return *this;
	}

	void instring::set(const sdstring &in) {
		memset(str, 0, m_size);
		m_size = in.size();
		free(str);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
	}

	void instring::set(const char* in) {
		memset(str, 0, m_size);
		m_size = strlen(in);
		free(str);
		str = static_cast<char*>(malloc(m_size + 1));
		wpos = str;
		memcpy(str, in, m_size);
		str[m_size] = 0;
	}

	instring instring::operator+(double V) const
	{
		sdstring temp = *this;
		std::ostringstream o;
		o << std::setprecision(JSON_NUMBER_PRECISION) << V;
		temp.append(o.str().c_str());
		return temp;
	}

	instring instring::operator+(sdstring& V) const
	{
		sdstring temp = *this;
		temp.append(V);
		return temp;
	}

	instring instring::operator+(const char* V) const
	{
		sdstring temp = *this;
		temp.append(V);
		return temp;
	}

	sdstring instring::Str() const
	{
		return sdstring(str);
	}

	sdstring instring::SoFar() const
	{
		return sdstring(str, wpos - str);
	}

	iterator value::begin() const
	{
		switch (myType) {
			case JSON_ARRAY:
				return iterator(arr->begin());

			case JSON_OBJECT:
				return iterator(obj->begin());

			default:
				return iterator();
		}
	}

	iterator value::end() const
	{
		switch (myType) {
			case JSON_ARRAY:
				return iterator(arr->end());

			case JSON_OBJECT:
				return iterator(obj->end());

			default:
				return iterator();
		}
	}

	reverse_iterator value::rbegin() const
	{
		switch (myType) {
			case JSON_ARRAY:
				return reverse_iterator(arr->rbegin());

			case JSON_OBJECT:
				return reverse_iterator(obj->rbegin());

			default:
				return reverse_iterator();
		}
	}

	reverse_iterator value::rend() const
	{
		switch (myType) {
			case JSON_ARRAY:
				return reverse_iterator(arr->rend());

			case JSON_OBJECT:
				return reverse_iterator(obj->rend());

			default:
				return reverse_iterator();
		}
	}

	iterator value::find(size_t index) const
	{
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			if (debug) {
				debug("json find: index %lu out of bounds", index);
			}
			return iterator();
		}
		if (myType == JSON_ARRAY) {
			if (index < arr->size())
				return arr->begin() + (long)index;
		}
		return iterator();
	}

	iterator value::find(const sdstring& index) const
	{
		if (myType == JSON_OBJECT) {
			return obj->find(index);
		}
		return iterator();
	}

	iterator value::find(const char* index) const
	{
		if (myType == JSON_OBJECT) {
			return obj->find(index);
		}
		return iterator();
	}

	reverse_iterator value::rfind(size_t index) const {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			if (debug) {
				debug("json rfind: index %lu out of bounds", index);
			}
			return reverse_iterator();
		}
		return reverse_iterator(find(index));
	}

	reverse_iterator value::rfind(const sdstring& index) const {
		iterator it = find(index);
		if (it == end()) {
			return reverse_iterator();
		}
		return reverse_iterator(++it);
	}

	reverse_iterator value::rfind(const char* index) const {
		iterator it = find(index);
		if (it == end()) {
			return reverse_iterator();
		}
		return reverse_iterator(++it);
	}

	bool value::boolean() const {
		switch (myType) {
		case JSON_VOID:
		case JSON_NULL:
		default:
			return false;
		case JSON_BOOLEAN:
			return m_boolean;
		case JSON_NUMBER:
			return m_number != 0.0;
		case JSON_STRING:
			return str.size() > 0;
		case JSON_ARRAY:
			return !arr->empty();
		case JSON_OBJECT:
			return !obj->empty();
		}
	}

	sdstring & value::makeStringFromNumber(sdstring & in, int iPlaces, double temp)
	{
		if (iPlaces >= 0) {
			temp = round((temp * pow(10, iPlaces))) / pow(10, iPlaces);
		}
		std::ostringstream s;
		s << std::setprecision(JSON_NUMBER_PRECISION) << temp;
		in = s.str().c_str();
		if (iPlaces >= 0) {
			size_t p = in.find('.');
			if (p != sdstring::npos) {
				size_t places = in.size() - p - 1;
				if (places < (size_t)iPlaces) {
					in.append((size_t)iPlaces - places, '0');
				}
			} else {
				if (iPlaces > 0) {
					in.append(".");
					in.append((size_t)iPlaces, '0');
				}
			}
		}
		return in;
	}

	size_t value::psize(size_t depth, bool bPretty) const
	{
		switch (isA()) {
		default:
		case JSON_VOID:
			return 0;

		case JSON_NULL:
			return 4;

		case JSON_BOOLEAN:
			if (m_boolean)
				return 4;
			else
				return 5;

		case JSON_NUMBER: {
			if (str.empty()) {
				sdstring s;
				makeStringFromNumber(s, m_places, m_number);
				return s.size();
			} else {
				return str.size();
			}
		}

		case JSON_STRING:
			return esize(str) + 2;

		case JSON_ARRAY:
			arr->resize(size());
			return arr->psize(depth + 1, bPretty);

		case JSON_OBJECT:
			return obj->psize(depth + 1, bPretty);
		}
	}

	void value::cprint(MovingCharPointer& ptr, size_t depth, bool bPretty) const
	{
		switch (isA()) {
		default:
		case JSON_VOID:
			break;

		case JSON_NULL:
			ptr.set("null", 4);
			break;

		case JSON_BOOLEAN:
			if (m_boolean) {
				ptr.set("true", 4);
			} else {
				ptr.set("false", 5);
			}
			break;

		case JSON_NUMBER: {
			if (str.empty()) {
				sdstring s;
				makeStringFromNumber(s, m_places, m_number);
				ptr.set(s.c_str(), s.size());
			} else {
				ptr.set(str.c_str(), str.size());
			}

			break;
		}

		case JSON_STRING: {
			ptr.set('\"');
			escape(ptr, str);
			ptr.set('\"');
			break;
		}

		case JSON_ARRAY:
			arr->cprint(ptr, depth + 1, bPretty);
			break;

		case JSON_OBJECT:
			obj->cprint(ptr, depth + 1, bPretty);
			break;
		}
	}

	void makeDepth(MovingCharPointer& ptr, size_t dep) {
		for (size_t i = 0; i < dep; i++) {
			ptr.set('\t');
		}
	}

	size_t array::psize(size_t depth, bool bPretty) const
	{
		size_t ret = 0;

		if (bPretty && !myVec::empty())
			ret += 2;
		else
			ret++;
		const_iterator it;
		for (it = begin(); it < end(); ++it) {
			if (it > begin()) {
				if (bPretty) {
					ret += 2;
				} else {
					ret++;
				}
			}
			if (bPretty)
				ret += depth;
			if ((*it).isA() == JSON_VOID) {
				ret += 4;
			} else {
				ret += it->psize(depth, bPretty);
			}
		}
		if (bPretty && !myVec::empty()) {
			ret++;
			ret += depth - 1;
		}
		ret++;
		return ret;
	}

	void array::cprint(MovingCharPointer& ptr, size_t depth, bool bPretty) const
	{
		if (bPretty && !myVec::empty()) {
			ptr.set("[\n", 2);
		} else {
			ptr.set('[');
		}
		const_iterator it;
		for (it = begin(); it < end(); ++it) {
			if (it > begin()) {
				if (bPretty) {
					ptr.set(",\n", 2);
				} else {
					ptr.set(',');
				}
			}
			if (bPretty) {
				makeDepth(ptr, depth);
			}
			if ((*it).isA() == JSON_VOID) {
				ptr.set("null", 4);
			} else {
				it->cprint(ptr, depth, bPretty);
			}
		}
		if (bPretty && !myVec::empty()) {
			ptr.set('\n');
			makeDepth(ptr, depth - 1);
		}
		ptr.set(']');
	}

	size_t object::psize(size_t depth, bool bPretty) const
	{
		size_t ret = 0;

		if (bPretty && !myMap::empty())
			ret += 2;
		else
			ret++;
		const_iterator it;
		for (it = begin(); it != end(); ++it) {
			if (it->second.isA() != JSON_VOID) {
				if (it != begin()) {
					if (bPretty)
						ret += 2;
					else
						ret++;
				}
				if (bPretty)
					ret += depth + esize(it->first) + 4;
				else
					ret += esize(it->first) + 3;
				ret += it->second.psize(depth, bPretty);
			}
		}
		if (bPretty && !myMap::empty()) {
			ret += depth;
		}
		ret++;

		return ret;
	}

	void object::cprint(MovingCharPointer& ptr, size_t depth, bool bPretty) const
	{
		if (bPretty && !myMap::empty()) {
			ptr.set("{\n", 2);
		} else {
			ptr.set('{');
		}
		const_iterator it;
		bool bStarted = false;
		for (it = begin(); it != end(); ++it) {
			if (it->second.isA() != JSON_VOID) {
				if (bStarted) {
					if (bPretty) {
						ptr.set(",\n", 2);
					} else {
						ptr.set(',');
					}
				} else {
					bStarted = true;
				}
				if (bPretty) {
					makeDepth(ptr, depth);
					ptr.set('\"');
					escape(ptr, it->first);
					ptr.set("\": ", 3);
				} else {
					ptr.set('\"');
					escape(ptr, it->first);
					ptr.set("\":", 2);
				}
				it->second.cprint(ptr, depth, bPretty);
			}
		}
		if (bPretty && !myMap::empty()) {
			ptr.set('\n');
			makeDepth(ptr, depth - 1);
		}
		ptr.set('}');
	}

	value value::merge(value& V) {
		value retVal;

		if (isA(JSON_OBJECT) && V.isA(JSON_OBJECT)) {
			retVal = *this;
			object::iterator it;
			for (it = V.obj->begin(); it != V.obj->end(); ++it) {
				sdstring index = it->first;
				retVal[index] = retVal[index].merge(it->second);
			}
		} else if (isA(JSON_ARRAY) && V.isA(JSON_ARRAY)) {
			retVal = *this;
			arr->insert(arr->end(), V.arr->begin(), V.arr->end());
		} else if (isA(JSON_ARRAY) && V.isA(JSON_OBJECT)) {
			retVal = *this;
			retVal[retVal.size()] = V;
		} else if (isA(JSON_OBJECT) && V.isA(JSON_ARRAY)) {
			retVal = V;
			retVal[retVal.size()] = *this;
		} else {
			retVal = V;
		}

		return retVal;
	}

	void value::erase(size_t index) {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			if (debug) {
				debug("json erase: index %lu out of bounds", index);
			}
			return;
		}
		if (myType == JSON_ARRAY) {
			if (index < arr->size()) {
				arr->erase(arr->begin() + (long)index);
			}
		}
	}

	// size_t value::erase(const sdstring &index) {
	// 	return erase(index.c_str());
	// }

	size_t value::erase(const sdstring& index) {
		if (myType == JSON_OBJECT) {
			myMap::iterator it;
			it = obj->find(index);
			if (it != obj->end()) {
				obj->erase(it);
				return 1;
			}
		}
		return 0;
	}

	iterator value::erase(iterator it) {
		if (it.IsArray() && arr) {
			return arr->erase(it.arr());
		} else if (!it.IsArray() && !it.Neither() && obj) {
			return obj->erase(it.obj());
		}
		if (arr != NULL) {
			return arr->end();
		} else if (obj != NULL) {
			return obj->end();
		}
		return iterator();
	}

	iterator value::erase(iterator first, iterator last)
	{
		if (first.IsArray() && last.IsArray() && arr) {
			arr->erase(first.arr(), last.arr());
		} else if (!first.IsArray() && !last.IsArray() && !first.Neither() && !last.Neither() && obj) {
			obj->erase(first.obj(), last.obj());
		}
		return iterator();
	}

	bool value::exists(size_t index) {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			if (debug) {
				debug("json exists: index %lu out of bounds", index);
			}
			return false;
		}
		if (isA(JSON_ARRAY) && arr != NULL) {
			if (arr->empty()) {
				return false;
			}
			if (index < arr->size()) {
				return true;
			}
		}
		return false;
	}

	// bool value::exists(const sdstring &index) {
	// 	return exists(index.c_str());
	// }

	bool value::exists(const sdstring& index) {
		if (isA(JSON_OBJECT) && obj != NULL) {
			if (obj->empty()) {
				return false;
			}
			iterator it = obj->find(index);
			if (it != obj->end()) {
				switch((*it).isA()) {
					case JSON_NULL:
					case JSON_BOOLEAN:
					case JSON_NUMBER:
					case JSON_STRING:
						return true;
					case JSON_ARRAY:
					case JSON_OBJECT:
						return !(*it).empty();
					default:
					case JSON_VOID:
						return false;
				}
			}
		}
		return false;
	}

	iterator value::insert(size_t index, value& V)
	{
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			if (debug) {
				debug("json insert: index %lu out of bounds", index);
			}
			return iterator();
		}
		if (myType != JSON_ARRAY) {
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			switch (myType) {
				case JSON_OBJECT:
					delete obj;
					obj = NULL;
					break;

				case JSON_ARRAY:
					delete arr;
					arr = NULL;
					break;

				default:
					break;
			}
			myType = JSON_ARRAY;
			if (!str.empty())
				str.clear();
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
		}
		if (index <= arr->size()) {
			return iterator(arr->insert(arr->begin() + (long)index, V));
		}
		return iterator();
	}

	iterator value::insert(const sdstring& index, value& V)
	{
		if (myType != JSON_OBJECT) {
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			myType = JSON_OBJECT;
			if (!str.empty())
				str.clear();
			obj = new object();
			if (pParentObject) {
				obj->setParentObject(pParentObject);
			} else if (pParentArray) {
				obj->setParentArray(pParentArray);
			}
			delete arr;
			arr = NULL;
		}
		return iterator(obj->insert(obj->end(), std::pair<sdstring, value>(index, V)));
	}

	// iterator value::insert(const char* index, value V)
	// {
	// 	if (myType != JSON_OBJECT) {
	// 		m_number = 0;
	// 		m_places = -1;
	// 		m_boolean = false;
	// 		myType = JSON_OBJECT;
	// 		if (!str.empty())
	// 			str.clear();
	// 		obj = new object();
	// 		if (pParentObject) {
	// 			obj->setParentObject(pParentObject);
	// 		} else if (pParentArray) {
	// 			obj->setParentArray(pParentArray);
	// 		}
	// 		delete arr;
	// 		arr = NULL;
	// 	}
	// 	return iterator(obj->insert(obj->end(), std::pair<sdstring, value>(index, V)));
	// }

	iterator value::insert(iterator position, value& V) {
		if (position.IsArray() && myType == JSON_ARRAY) {
			return iterator(arr->insert(position.arr(), V));
#if defined _USE_ADDED_ORDER_
		} else if (!position.IsArray() && !position.Neither() && myType == JSON_OBJECT && V.myType == JSON_OBJECT) {
			return iterator(obj->insert(position.obj(), V.begin().obj(), V.end().obj()));
#else
		} else if (myType == JSON_OBJECT && V.myType == JSON_OBJECT) {
			obj->insert(V.begin().obj(), V.end().obj());
			return obj->find(V.begin().key().string());
#endif
		} else {
			return iterator();
		}
	}

	iterator value::insert(iterator position, const sdstring &key, value &V) {
		if (!position.IsArray() && !position.Neither() && myType == JSON_OBJECT) {
			return iterator(obj->insert(position.obj(), std::pair<sdstring, value>(key, V)));
		} else {
			return iterator();
		}
	}

	void value::insert(iterator position, iterator first, iterator last) {
		if (position.IsArray() && first.IsArray() && last.IsArray()) {
			if (myType != JSON_ARRAY || arr == NULL) {
				m_number = 0;
				m_places = -1;
				m_boolean = false;
				switch (myType) {
					case JSON_OBJECT:
						delete obj;
						obj = NULL;
						break;

					case JSON_ARRAY:
						delete arr;
						arr = NULL;
						break;

					default:
						break;
				}
				myType = JSON_ARRAY;
				if (!str.empty())
					str.clear();

				arr = new array();
				if (pParentObject) {
					arr->setParentObject(pParentObject);
				} else if (pParentArray) {
					arr->setParentArray(pParentArray);
				}
			}
			arr->insert(position.arr(), first.arr(), last.arr());
#if defined _USE_ADDED_ORDER_
		} else if (!position.IsArray() && !position.Neither() && !first.IsArray() && !first.Neither() && !last.IsArray() && !last.Neither()) {
#else
		} else if (!first.IsArray() && !first.Neither() && !last.IsArray() && !last.Neither()) {
#endif
			if (myType != JSON_OBJECT || obj == NULL) {
				m_number = 0;
				m_places = -1;
				m_boolean = false;
				myType = JSON_OBJECT;
						if (arr) {
								delete arr;
							 arr = NULL;
						}
				if (!str.empty())
					str.clear();
				obj = new object();
				if (pParentObject) {
					obj->setParentObject(pParentObject);
				} else if (pParentArray) {
					obj->setParentArray(pParentArray);
				}
			}
#if defined _USE_ADDED_ORDER_
			obj->insert(position.obj(), first.obj(), last.obj());
#else
			obj->insert(first.obj(), last.obj());
#endif
		}
	}

	void value::insert(iterator first, iterator last) {
		if (first.IsArray() && last.IsArray()) {
			if (myType != JSON_ARRAY) {
				m_number = 0;
				m_places = -1;
				m_boolean = false;
				switch (myType) {
					case JSON_OBJECT:
						delete obj;
						obj = NULL;
						break;

					case JSON_ARRAY:
						delete arr;
						arr = NULL;
						break;

					default:
						break;
				}
				myType = JSON_ARRAY;
				if (!str.empty())
					str.clear();

				arr = new array();
				if (pParentObject) {
					arr->setParentObject(pParentObject);
				} else if (pParentArray) {
					arr->setParentArray(pParentArray);
				}
			}
			arr->insert(arr->end(), first.arr(), last.arr());
		} else if (!first.IsArray() && !first.Neither() && !last.IsArray() && !last.Neither()) {
			if (myType != JSON_OBJECT) {
				m_number = 0;
				m_places = -1;
				m_boolean = false;
				myType = JSON_OBJECT;
				if (!str.empty())
					str.clear();
				obj = new object();
				if (pParentObject) {
					obj->setParentObject(pParentObject);
				} else if (pParentArray) {
					obj->setParentArray(pParentArray);
				}
				delete arr;
				arr = NULL;
			}
			obj->insert(first.obj(), last.obj());
		}
	}

	const value parse(instring& inputString, bool* bFailed) {
		value ret;
		objectParse(ret, inputString, bFailed);
		return ret;
	}

    std::string value::getKey(size_t index) {
		assert(i64(index) >= 0);
		if (isA() == JSON_OBJECT) {
			if (obj == NULL) {
				obj = new object();
				if (pParentObject) {
					obj->setParentObject(pParentObject);
				} else if (pParentArray) {
					obj->setParentArray(pParentArray);
				}
				delete arr;
				arr = NULL;
			}
			size_t i = 0;
			for (object::iterator it = obj->begin(); it != obj->end(); ++it) {
				if (i++ == index) {
					return it->first.c_str();
				}
			}
			return c_str();
		} else {
			return c_str();
		}
	}

	value::DEBUGPTR value::debug = NULL;

	value::value(const value& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(V.myType), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

	value::value(value&& V)
	{
		std::swap(m_number, V.m_number);
		std::swap(m_places, V.m_places);
		std::swap(m_boolean, V.m_boolean);

		str = std::move(V.str);

		myType = V.myType;

		switch (myType) {
			default:
				break;

			case JSON_OBJECT:
				obj = std::move(V.obj);
				break;

			case JSON_ARRAY:
				arr = std::move(V.arr);
				break;
		}

		pParentObject = NULL;
		pParentArray = NULL;

		V.myType = JSON_VOID;
	}

	value::value(const document& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(V.myType), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

#if defined SUPPORT_ORDERED_JSON && !defined _USE_ADDED_ORDER_
	value::value(const ojson::value& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(JSONTypes((int)V.myType)), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

	value::value(const ojson::document& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(JSONTypes((int)V.myType)), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

#elif defined _USE_ADDED_ORDER_

	value::value(const json::value& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(JSONTypes((int)V.myType)), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{

		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

	value::value(const json::document& V) : m_number(V.m_number), m_places(V.m_places), m_boolean(V.m_boolean), str(V.str), myType(JSONTypes((int)V.myType)), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		if (myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}
	}

	array::array(const json::array& V)
	: myVec(V.begin(), V.end()) {
		bNotEmpty = V.bNotEmpty;
		pParentArray = NULL;
		pParentObject = NULL;
	}
	array::array(const json::array* V)
	: myVec(V->begin(), V->end()) {
		bNotEmpty = V->bNotEmpty;
		pParentArray = NULL;
		pParentObject = NULL;
	}

	object::object(const json::object& V)
	// : myMap((const std::map<sdstring, json::value>)V)
	{
        std::map<sdstring, json::value, std::less<sdstring>, secure_delete_allocator<std::pair<const sdstring, json::value>>> in = (std::map<sdstring, json::value, std::less<sdstring>, secure_delete_allocator<std::pair<const sdstring, json::value>>>)V;
		for (auto it = in.begin(); it != in.end(); ++it) {
			(*this).insert(this->end(), *it);
		}

		bNotEmpty = V.bNotEmpty;
		pParentArray = NULL;
		pParentObject = NULL;
	}
	object::object(const json::object* V)
	// : myMap(V)
	{
		std::map<sdstring, json::value, std::less<sdstring>, secure_delete_allocator<std::pair<const sdstring, json::value>>> in = (std::map<sdstring, json::value, std::less<sdstring>, secure_delete_allocator<std::pair<const sdstring, json::value>>>)*V;
		for (auto it = in.begin(); it != in.end(); ++it) {
			(*this).insert(this->end(), *it);
		}

		bNotEmpty = V->bNotEmpty;
		pParentArray = NULL;
		pParentObject = NULL;
	}

#endif

	const char* value::typeName(JSONTypes type)
	{
		switch(type) {
			default:
			case JSON_VOID:
				return("Void (Will not output)");

			case JSON_NULL:
				return("NULL");

			case JSON_BOOLEAN:
				return("Boolean");

			case JSON_NUMBER:
				return("Number");

			case JSON_STRING:
				return("String");

			case JSON_ARRAY:
				return("Array");

			case JSON_OBJECT:
				return("Object");
		}
	}

	value& value::operator=(const value& V) {
		if (this == &V)
			return *this;

		if (debug) {
			if (myType != V.myType) {
				switch(myType) {
						// case JSON_NULL:
						// 	debug("json operator= changed type from NULL to %s", typeName(V.myType).c_str());
						// 	break;

					case JSON_BOOLEAN:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Boolean %i to %s", m_boolean, typeName(V.myType));
						}
						break;

					case JSON_NUMBER:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Number %f to %s", m_number, typeName(V.myType));
						}
						break;

					case JSON_STRING:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from String '%s' to %s", str.c_str(), typeName(V.myType));
						}
						break;

					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(V.myType), this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(V.myType), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}

		m_number = V.m_number;
		m_places = V.m_places;
		m_boolean = V.m_boolean;

		str.assign(V.str);


		if (myType != V.myType) {
			switch (myType) {
				case JSON_OBJECT:
					delete obj;
					obj = NULL;
					break;

				case JSON_ARRAY:
					delete arr;
					arr = NULL;
					break;

				default:
					break;
			}

			myType = V.myType;
			if (V.myType == JSON_OBJECT) {
				obj = new object(V.obj);
			} else if (V.myType == JSON_ARRAY) {
				arr = new array(V.arr);
			}
		} else {
			if (V.myType == JSON_OBJECT) {
				*obj = *(V.obj);
			} else if (V.myType == JSON_ARRAY) {
				*arr = *(V.arr);
			}
		}

		if (myType != JSON_VOID) {
			if (pParentObject) {
				pParentObject->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentObject(pParentObject);
				} else if (myType == JSON_OBJECT) {
					obj->setParentObject(pParentObject);
				}
			} else if (pParentArray) {
				pParentArray->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentArray(pParentArray);
				} else if (myType == JSON_OBJECT) {
					obj->setParentArray(pParentArray);
				}
			}
		}
		// m_key = V.m_key;
		return *this;
	}

	value& value::operator=(value&& V) {
		if (debug) {
			if (myType != V.myType) {
				switch(myType) {
						// case JSON_NULL:
						// 	debug("json operator= changed type from NULL to %s", typeName(V.myType).c_str());
						// 	break;

					case JSON_BOOLEAN:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Boolean %i to %s", m_boolean, typeName(V.myType));
						}
						break;

					case JSON_NUMBER:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Number %f to %s", m_number, typeName(V.myType));
						}
						break;

					case JSON_STRING:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from String '%s' to %s", str.c_str(), typeName(V.myType));
						}
						break;

					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(V.myType), this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(V.myType), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}

		m_number = std::move(V.m_number);
		m_places = std::move(V.m_places);
		m_boolean = std::move(V.m_boolean);

		str = std::move(V.str);

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;
			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		myType = std::move(V.myType);

		obj = std::move(V.obj);

		if (myType != JSON_VOID) {
			if (pParentObject) {
				pParentObject->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentObject(pParentObject);
				} else if (myType == JSON_OBJECT) {
					obj->setParentObject(pParentObject);
				}
			} else if (pParentArray) {
				pParentArray->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentArray(pParentArray);
				} else if (myType == JSON_OBJECT) {
					obj->setParentArray(pParentArray);
				}
			}
		}

		// V.m_number = 0;
		// V.m_places = -1;
		// V.m_boolean = false;
		V.myType = JSON_VOID;

		// V.obj = NULL;
		// V.arr = NULL;

		V.pParentObject = NULL;
		V.pParentArray = NULL;

		return *this;
	}

	value::value(bool V) : m_number((double)V), m_places(-1), m_boolean(V), myType(JSON_BOOLEAN), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		// m_number = (double)V;
		// m_places = -1;
		// m_boolean = !(V == 0);

		// myType = JSON_BOOLEAN;
		// obj = NULL;
		// arr = NULL;
		// pParentObject = NULL;
		// pParentArray = NULL;
	}

	value::value(const char* V) : m_number(0), m_places(-1), m_boolean(false), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		// m_number = 0;
		// m_places = -1;
		// m_boolean = false;

		if (V) {
			str.assign(V);
			myType = JSON_STRING;
		} else {
			myType = JSON_NULL;
		}

		// obj = NULL;
		// arr = NULL;
		// pParentObject = NULL;
		// pParentArray = NULL;
	}

	value::value(char* V) : m_number(0), m_places(-1), m_boolean(false), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		// m_number = 0;
		// m_places = -1;
		// m_boolean = false;

		if (V) {
			str.assign(V);
			myType = JSON_STRING;
		} else {
			myType = JSON_NULL;
		}

		// obj = NULL;
		// arr = NULL;
		// pParentObject = NULL;
		// pParentArray = NULL;
	}
	value::value(const sdstring& V) : m_number(0), m_places(-1), m_boolean(false), str(V), myType(JSON_STRING), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
	// 	m_number = 0;
	// 	m_places = -1;
	// 	m_boolean = false;
	// 	str.assign(V);
	// 	myType = JSON_STRING;
	// 	obj = NULL;
	// 	arr = NULL;
	// 	pParentObject = NULL;
	// 	pParentArray = NULL;
	}

	value::value(sdstring&& V) : m_number(0), m_places(-1), m_boolean(false), myType(JSON_STRING), obj(nullptr), pParentObject(nullptr), pParentArray(nullptr)
	{
		std::swap(str, V);
	// 	m_number = 0;
	// 	m_places = -1;
	// 	m_boolean = false;
	// 	str.assign(V);
	// 	myType = JSON_STRING;
	// 	obj = NULL;
	// 	arr = NULL;
	// 	pParentObject = NULL;
	// 	pParentArray = NULL;
	}
/*
	value & value::operator=(const sdstring& V)
	{
		if (debug) {
			if (myType != JSON_STRING) {
				switch(myType) {
					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(JSON_STRING), this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(JSON_STRING), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;
			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		str.assign(V);
		myType = JSON_STRING;
		if (pParentObject) {
			pParentObject->setNotEmpty();
		} else if (pParentArray) {
			pParentArray->setNotEmpty();
		}
		return *this;
	}

	value & value::operator=(sdstring&& V)
	{
		if (debug) {
			if (myType != JSON_STRING) {
				switch(myType) {
					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(JSON_STRING), this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(JSON_STRING), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;
			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		std::swap(str, V);
		myType = JSON_STRING;
		if (pParentObject) {
			pParentObject->setNotEmpty();
		} else if (pParentArray) {
			pParentArray->setNotEmpty();
		}
		return *this;
	}
*/
//	value::value(sdstring V) {
//		m_number = 0;
//		m_places = -1;
//		m_boolean = false;
//		str.assign(V);
//		myType = JSON_STRING;
//		obj = NULL;
//		arr = NULL;
//		pParentObject = NULL;
//		pParentArray = NULL;
//	}
//#if !defined USE_STD_STRING
//	value::value(const std::string &V) {
//		m_number = 0;
//		m_places = -1;
//		m_boolean = false;
//		str.assign(V.c_str());
//		myType = JSON_STRING;
//		obj = NULL;
//		arr = NULL;
//		pParentObject = NULL;
//		pParentArray = NULL;
//	}
//#endif
	value::value(object& V) {
		m_number = 0;
		m_places = -1;
		m_boolean = false;

		myType = JSON_OBJECT;
		obj = new object(V);
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}
	value::value(array& V) {
		m_number = 0;
		m_places = -1;
		m_boolean = false;

		myType = JSON_ARRAY;
		obj = NULL;
		arr = new array(V);
		pParentObject = NULL;
		pParentArray = NULL;
	}

	int value::isA() const
	{
		switch(myType) {
			case JSON_ARRAY:
			{
				if (!arr->notEmpty() && arr->empty()) {
					return JSON_VOID;
				}
				break;
			}
			case JSON_OBJECT:
			{
				if (!obj->notEmpty() && obj->empty()) {
					return JSON_VOID;
				}
				break;
			}
			default:
				break;
		}
		return myType;
	}

	value::~value() {
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				break;

			case JSON_ARRAY:
				delete arr;
				break;

			default:
				break;
		}
	}

	value& value::at(size_t index)
	{
//		assert(i64(index) >= 0);
		if (index >= std::numeric_limits<size_t>::max() / 2) {
			if (debug) {
				debug("json at: index %lu out of bounds", index);
			}
			return *this;
		}
		if (myType == JSON_OBJECT) {
			if (index < obj->size()) {
#if defined _USE_ADDED_ORDER_
				return (*obj)[index];
#else
				size_t iMyIndex = 0;
				for (object::iterator it = obj->begin(); it != obj->end(); ++it) {
					if (iMyIndex++ == index) {
						return it->second;
					}
				}
#endif
			}
		} else if (myType == JSON_ARRAY) {
			return this[index];
		}
		return *this;
	}

	value& value::emptyArray()
	{
		if (myType != JSON_VOID) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json emptyArray() changed type from NULL to Array.");
					// 	break;

					case JSON_BOOLEAN:
						debug("json emptyArray() changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json emptyArray() changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json emptyArray() changed type from String '%s' to Array.", str.c_str());
						break;

					case JSON_ARRAY:
						debug("json emptyArray() cleared Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json emptyArray() changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;

			str.clear();
			switch (myType) {
				case JSON_OBJECT:
					delete obj;
					// obj = NULL;
					break;

				case JSON_ARRAY:
					delete arr;
					// arr = NULL;
					break;

				default:
					break;
			}
		}
		myType = JSON_ARRAY;
		arr = new array();
		if (pParentObject) {
			arr->setParentObject(pParentObject);
		} else if (pParentArray) {
			arr->setParentArray(pParentArray);
		}
		arr->setNotEmpty();
		return *this;
	}

	value& value::emptyObject()
	{
		if (myType != JSON_VOID) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json emptyObject() changed type from NULL to Object.");
					// 	break;

					case JSON_BOOLEAN:
						debug("json emptyObject() changed type from Boolean to Object.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json emptyObject() changed type from Number %f to Object.", m_number);
						break;

					case JSON_STRING:
						debug("json emptyObject() changed type from String '%s' to Object.", str.c_str());
						break;

					case JSON_ARRAY:
						debug("json emptyObject() changed type from Array to Object, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json emptyObject() cleared Object, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;

			str.clear();

			switch (myType) {
				case JSON_OBJECT:
					delete obj;
					// obj = NULL;
					break;

				case JSON_ARRAY:
					delete arr;
					// arr = NULL;
					break;

				default:
					break;
			}
		}
		myType = JSON_OBJECT;
		obj = new object();
		if (pParentObject) {
			obj->setParentObject(pParentObject);
		} else if (pParentArray) {
			obj->setParentArray(pParentArray);
		}
		obj->setNotEmpty();
		return *this;
	}

	value & value::toArray()
	{
		if (myType == JSON_ARRAY) {
			return *this;
		}
		value temp = *this;
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		str.clear();
		if (myType == JSON_OBJECT) {
			delete obj;
			// obj = NULL;
		}
		myType = JSON_ARRAY;
		arr = new array();
		if (pParentObject) {
			arr->setParentObject(pParentObject);
		} else if (pParentArray) {
			arr->setParentArray(pParentArray);
		}
		temp.setParentArray(arr);
		temp.setParentObject(NULL);
		if (!temp.isA(JSON_VOID)) {
			(*this)[0] = temp;
		}
		return *this;
	}

	value & value::toObject(const sdstring& key)
	{
		return toObject(key.c_str());
	}

	value & value::toObject(const char * key)
	{
		if (myType == JSON_OBJECT) {
			return *this;
		}
		value temp = *this;
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		str.clear();
		if (myType == JSON_ARRAY) {
			delete arr;
			arr = NULL;
		}
		myType = JSON_OBJECT;
		obj = new object();
		if (pParentObject) {
			obj->setParentObject(pParentObject);
		} else if (pParentArray) {
			obj->setParentArray(pParentArray);
		}
		if (!temp.isA(JSON_VOID)) {
			(*this)[key] = temp;
		}
		return *this;
	}

	value & value::toString(int iDecimalPlaces)
	{
		if (iDecimalPlaces > JSON_NUMBER_PRECISION) {
			iDecimalPlaces = JSON_NUMBER_PRECISION;
		}
		m_places = iDecimalPlaces;
		if (myType == JSON_STRING) {
			return *this;
		} else if (myType == JSON_NUMBER) {
			str.clear();
		}
		value temp = *this;
		temp.str.clear();
		m_number = 0;
		m_boolean = false;

		if (iDecimalPlaces >= 0 && temp.isA(JSON_NUMBER)) {
			temp.m_number = round((temp.m_number * pow(10, iDecimalPlaces))) / pow(10, iDecimalPlaces);
		}
		str = temp._sdstring();
		if (iDecimalPlaces >= 0 && temp.isA(JSON_NUMBER)) {
			size_t p = str.find('.');
			if (p != sdstring::npos) {
				size_t places = str.size() - p - 1;
				if (places < (size_t)iDecimalPlaces) {
					str.append((size_t)iDecimalPlaces - places, '0');
				}
			} else {
				if (iDecimalPlaces > 0) {
					str.append(".");
					str.append((size_t)iDecimalPlaces, '0');
				}
			}
		}
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = JSON_STRING;

		return *this;
	}

	value & value::toString()
	{
		if (myType == JSON_STRING) {
			return *this;
		}
		value temp = *this;
		m_number = 0;
		m_boolean = false;

		str = temp._sdstring();

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = JSON_STRING;

		return *this;
	}

	value & value::toNumber()
	{
		str.clear();
		if (myType == JSON_NUMBER) {
			return *this;
		}
		value temp = *this;
		m_number = temp.number();
		m_boolean = false;

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = JSON_NUMBER;

		return *this;
	}

	value & value::fixedDecimal(int iPlaces)
	{
		if (iPlaces > JSON_NUMBER_PRECISION) {
			iPlaces = JSON_NUMBER_PRECISION;
		}
		toNumber();
		m_places = iPlaces;

		return *this;
	}

	value & value::toBool()
	{
		if (myType == JSON_BOOLEAN) {
			return *this;
		}
		value temp = *this;
		m_number = 0;
		m_places = -1;
		m_boolean = temp.boolean();

		str.clear();

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = JSON_BOOLEAN;

		return *this;
	}

	value & value::toNull()
	{
		if (myType == JSON_NULL) {
			return *this;
		}
		m_number = 0;
		m_places = -1;
		m_boolean = false;

		str.clear();

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = JSON_NULL;

		return *this;
	}

	value& value::operator[](size_t index) {
		assert(i64(index) >= 0);
		if (index >= std::numeric_limits<size_t>::max() / 2) {
			if (debug) {
				debug("json find: index %lu out of bounds", index);
			}
			return *this;
		}
		if (myType == JSON_ARRAY) {
			if (index < arr->size()) {
				value & ret = arr->at(index);
				ret.setParentArray(arr);
				return ret;
			} else {
				arr->resize(index + 1);
				value & ret = arr->at(index);
				ret.setParentArray(arr);
				return ret;
			}
		} else if (myType != JSON_VOID) {
			if (index == 0) {
				return *this;
			}
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json operator[](size_t index = %lu) changed type from NULL to Array.", index);
					// 	break;

					case JSON_BOOLEAN:
						debug("json operator[](size_t index = %lu) changed type from Boolean to Array.", index, m_boolean);
						break;

					case JSON_NUMBER:
						debug("json operator[](size_t index = %lu) changed type from Number %f to Array.", index, m_number);
						break;

					case JSON_STRING:
						debug("json operator[](size_t index = %lu) changed type from String '%s' to Array.", index, str.c_str());
						break;

					case JSON_OBJECT:
						debug("json operator[](size_t index = %lu) changed type from Object to Array, orphanning:\n%s\n", index, this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			str.clear();
			if (myType == JSON_OBJECT) {
				delete obj;
				// obj = NULL;
			}
		}

		myType = JSON_ARRAY;
		arr = new array();
		if (pParentObject) {
			arr->setParentObject(pParentObject);
		} else if (pParentArray) {
			arr->setParentArray(pParentArray);
		}
		arr->resize(index + 1);
		value & ret = arr->at(index);
		ret.setParentArray(arr);
		return ret;
	}

	// value& value::operator[](const sdstring& index) {
	// 	return operator[](index.c_str());
	// }

	value& value::operator[](const sdstring& index) {
		if (myType == JSON_OBJECT) {
			value& ret = obj->operator[](index);
			ret.setParentObject(obj);
			// ret.m_key.assign(index);
			return ret;
		} else if (myType != JSON_VOID) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json operator[](sdstring index = '%s') changed type from NULL to Object.", index.c_str());
					// 	break;

					case JSON_BOOLEAN:
						debug("json operator[](sdstring index = '%s') changed type from Boolean to Object.", index.c_str(), m_boolean);
						break;

					case JSON_NUMBER:
						debug("json operator[](sdstring index = '%s') changed type from Number %f to Object.", index.c_str(), m_number);
						break;

					case JSON_STRING:
						debug("json operator[](sdstring index = '%s') changed type from String '%s' to Object.", index.c_str(), str.c_str());
						break;

					case JSON_ARRAY:
						debug("json operator[](sdstring index = '%s') changed type from Array to Object(%s), orphanning:\n%s\n", index.c_str(), index.c_str(), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			str.clear();
			if (myType == JSON_ARRAY) {
				delete arr;
				// arr = NULL;
			}
		}
		myType = JSON_OBJECT;
		obj = new object();
		if (pParentObject) {
			obj->setParentObject(pParentObject);
		} else if (pParentArray) {
			obj->setParentArray(pParentArray);
		}
		value & ret = obj->operator[](index);
		ret.setParentObject(obj);
		// ret.m_key.assign(index);
		return ret;
	}

	void value::push_back(const value& val) {
		if (myType != JSON_ARRAY) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json push_back(value val) changed type from NULL to Array.");
					// 	break;

					case JSON_BOOLEAN:
						debug("json push_back(value val) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json push_back(value val) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json push_back(value val) changed type from String '%s' to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json push_back(value val) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			str.clear();
			if (myType == JSON_OBJECT) {
				delete obj;
				// obj = NULL;
			}
			myType = JSON_ARRAY;
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
		}
		arr->emplace_back(val);
		arr->back().setParentArray(arr);

		if (val.myType != JSON_VOID) {
			arr->setNotEmpty();
		}
	}

	void value::push_back(value&& val) {
		if (myType != JSON_ARRAY) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json push_back(value val) changed type from NULL to Array.");
					// 	break;

					case JSON_BOOLEAN:
						debug("json push_back(value val) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json push_back(value val) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json push_back(value val) changed type from String '%s' to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json push_back(value val) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			str.clear();
			if (myType == JSON_OBJECT) {
				delete obj;
				// obj = NULL;
			}
			myType = JSON_ARRAY;
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
		}
		arr->emplace_back(val);
		arr->back().setParentArray(arr);

		if (val.myType != JSON_VOID) {
			arr->setNotEmpty();
		}
	}

	void value::push_front(const value &val) {
		if (myType != JSON_ARRAY) {
			if (debug) {
				switch(myType) {
					// case JSON_NULL:
					// 	debug("json push_front(value val) changed type from NULL to Array.");
					// 	break;

					case JSON_BOOLEAN:
						debug("json push_front(value val) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json push_front(value val) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json push_front(value val) changed type from String '%s' to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json push_front(value val) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_places = -1;
			m_boolean = false;
			str.clear();
			if (myType == JSON_OBJECT) {
				delete obj;
				// obj = NULL;
			}
			arr = new array();
			myType = JSON_ARRAY;
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
		}
		arr->emplace_front(val);
		arr->front().setParentArray(arr);
		if (val.myType != JSON_VOID) {
			arr->setNotEmpty();
		}
	}

	value value::pop_back()
	{
		value ret;
		if (myType == JSON_ARRAY) {
			if (!arr->empty()) {
				ret = *(arr->rbegin());
				arr->pop_back();
			}
		}
		return ret;
	}

	value value::pop_front()
	{
		value ret;
		if (myType == JSON_ARRAY) {
			if (!arr->empty()) {
				ret = *(arr->begin());
				arr->pop_front();
			}
		}
		return ret;
	}

	void value::resize(size_t iCount) {
		if (myType == JSON_VOID) {
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
			myType = JSON_ARRAY;
		} else if (myType == JSON_ARRAY) {
			arr->resize(iCount);
		}
	}

	void value::resize(size_t iCount, value &val) {
		if (myType == JSON_VOID) {
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
			myType = JSON_ARRAY;
		} else if (myType == JSON_ARRAY) {
			arr->resize(iCount, val);
		}
	}

	bool value::pruneEmptyValues()
	{
		switch(myType) {
			default:
			case JSON_VOID:
				return false;

			case JSON_NULL:
				m_number = 0;
				m_places = -1;
				m_boolean = false;
				myType = JSON_VOID;
				str.clear();
				return false;

			case JSON_BOOLEAN:
				if (m_boolean == false) {
					m_number = 0;
					m_places = -1;
					myType = JSON_VOID;
					str.clear();
					return false;
				}
				return true;

			case JSON_NUMBER:
				if (m_number == 0.0 && m_places < 0) {
					m_places = -1;
					m_boolean = false;
					myType = JSON_VOID;
					str.clear();
					return false;
				}
				return true;

			case JSON_STRING:
				if (str.empty()) {
					m_number = 0;
					m_places = -1;
					m_boolean = false;
					myType = JSON_VOID;
					return false;
				}
				return true;

			case JSON_ARRAY:
			{
				bool bNotEmpty = false;
				for (reverse_iterator rit = (*this).rbegin(); rit != (*this).rend(); ++rit) {
					if ((*rit).isA(JSON_NULL) && bNotEmpty) {
						continue;  // NULLs are placeholders in arrays and only ones after the last non null value are pruned.
					}
					if ((*rit).pruneEmptyValues()) {
						bNotEmpty = true;
					}
				}
				if (bNotEmpty == false) {
					m_number = 0.0;
					m_places = -1;
					m_boolean = false;
					str.clear();
					switch (myType) {
						case JSON_OBJECT:
							delete obj;
							obj = NULL;
							break;

						case JSON_ARRAY:
							delete arr;
							arr = NULL;
							break;

						default:
							break;
					}
					myType = JSON_VOID;
				}
				return bNotEmpty;
			}
			case JSON_OBJECT:
			{
				bool bNotEmpty = false;
				for (value &val : *this) {
					if (val.pruneEmptyValues()) {
						bNotEmpty = true;
					}
				}
				if (bNotEmpty == false) {
					m_number = 0.0;
					m_places = -1;
					m_boolean = false;
					str.clear();
					switch (myType) {
						case JSON_OBJECT:
							delete obj;
							obj = NULL;
							break;

						case JSON_ARRAY:
							delete arr;
							arr = NULL;
							break;

						default:
							break;
					}
					myType = JSON_VOID;
				}
				return bNotEmpty;
			}
		}
	}

	bool value::compact()
	{
		switch(myType) {
			default:
				return true;

			case JSON_VOID:
				return false;

			case JSON_ARRAY:
			{
				bool bNotEmpty = false;
				size_t s = arr->size();
				for (auto rit = arr->rbegin(); rit != arr->rend() && s > 0; ++rit) {
					if ((*rit).compact()) {
						bNotEmpty = true;
						break;
					} else {
						s--;
					}
				}

				if (bNotEmpty == false) {
					m_number = 0.0;
					m_places = -1;
					m_boolean = false;
					str.clear();
					switch (myType) {
						case JSON_OBJECT:
							delete obj;
							obj = NULL;
							break;

						case JSON_ARRAY:
							delete arr;
							arr = NULL;
							break;

						default:
							break;
					}
					myType = JSON_VOID;
				} else {
					resize(s);
				}
				return bNotEmpty;
			}
			case JSON_OBJECT:
			{
				bool bNotEmpty = false;
				for (auto it = obj->begin(); it != obj->end();) {
					if ((*it).second.compact()) {
						bNotEmpty = true;
						++it;
					} else {
						auto it2 = it;
						++it2;
						obj->erase(it);
						it = it2;
					}
				}
				if (bNotEmpty == false) {
					m_number = 0.0;
					m_places = -1;
					m_boolean = false;
					str.clear();
					switch (myType) {
						case JSON_OBJECT:
							delete obj;
							obj = NULL;
							break;

						case JSON_ARRAY:
							delete arr;
							arr = NULL;
							break;

						default:
							break;
					}
					myType = JSON_VOID;
				}
				return bNotEmpty;
			}
		}
	}

	bool value::empty() const
	{
		switch (isA()) {
			case JSON_OBJECT:
				return obj->empty();
			case JSON_ARRAY:
			{
				return arr->empty();
			}

			case JSON_STRING:
			{
				return str.empty();
			}

			case JSON_NULL:
			case JSON_VOID:
				return true;

			default:
				return false;
		}
	}

	bool array::empty() const
	{
		if (/*!bNotEmpty && */myVec::empty()) {
			// bNotEmpty = false;
			return true;
		} else {
			return !bNotEmpty;
		}
	}

	void array::setNotEmpty()
	{
		if (bNotEmpty) {
			return;
		}
		bNotEmpty = true;

		if (pParentArray && pParentArray != this) {
			if (pParentArray->empty()) {
				pParentArray->setNotEmpty();
			}
		} else if (pParentObject) {
			if (pParentObject->empty()) {
				pParentObject->setNotEmpty();
			}
		}
	}

	void object::setParentArray(array * pSetTo)
	{
		pParentArray = pSetTo;
		if (bNotEmpty && pParentArray) {
			pParentArray->setNotEmpty();
		}
	}

	void object::setParentObject(object * pSetTo)
	{
		if (pSetTo == this) {
			return;
		}
		pParentObject = pSetTo;
		if (bNotEmpty && pParentObject) {
			pParentObject->setNotEmpty();
		}
	}

	void array::setParentArray(array * pSetTo)
	{
		if (pSetTo == this) {
			return;
		}
		pParentArray = pSetTo;
		if (bNotEmpty && pParentArray) {
			pParentArray->setNotEmpty();
		}
	}

	void array::setParentObject(object * pSetTo)
	{
		pParentObject = pSetTo;
		if (bNotEmpty && pParentObject) {
			pParentObject->setNotEmpty();
		}
	}

	bool object::empty() const
	{
		// for (const std::pair<sdstring, value> &pair: *this) {
		// 	if (!pair.second.empty()) {
		// 		return false;
		// 	}
		// }
		if (/*!bNotEmpty && */myMap::empty()) {
			// bNotEmpty = false;
			return true;
		} else {
			return !bNotEmpty;
		}
		// return myMap::empty() && !bNotEmpty;
	}
	void object::setNotEmpty()
	{
		if (bNotEmpty) {
			return;
		}
		bNotEmpty = true;
		if (pParentArray) {
			if (pParentArray->empty()) {
				pParentArray->setNotEmpty();
			}
		} else if (pParentObject && pParentObject != this) {
			if (pParentObject->empty()) {
				pParentObject->setNotEmpty();
			}
		}
	}

	void value::setParentObject(object* pSetTo)
	{
		pParentObject = pSetTo;
		if (myType == JSON_ARRAY) {
			arr->setParentObject(pSetTo);
		} else if (myType == JSON_OBJECT) {
			obj->setParentObject(pSetTo);
		}
	}

	void value::setParentArray(array* pSetTo)
	{
		pParentArray = pSetTo;
		if (myType == JSON_ARRAY) {
			arr->setParentArray(pSetTo);
		} else if (myType == JSON_OBJECT) {
			obj->setParentArray(pSetTo);
		}
	}

	size_t value::size() const
	{
		switch (myType) {
		case JSON_ARRAY:
		{
			size_t ret = arr->size();
			auto ritend = rend();
			for (reverse_iterator it = rbegin(); it != ritend; ++it) {
                if ((*it).isA(JSON_VOID)) {
					ret--;
				} else {
					break;
				}
			}
			return ret;
		}

		case JSON_OBJECT:
		{
			size_t ret = 0;
			auto itend = this->end();
			for (iterator it = this->begin(); it != itend; ++it) {
                if (!(*it).isA(JSON_VOID)) {
					ret++;
				}
			}
			return ret;
		}

		case JSON_VOID:
		case JSON_NULL:
			return 0;

		default:
			return 1;
		}
	}

	size_t value::length()
	{
		return string().size();
	}

	size_t value::arraySize()
	{
		if (!isA(JSON_ARRAY))
			toArray();
		return size();
	}

	value value::simpleSearch(value& searchFor, bool bSubStr) {
		value retVal;

		if (myType == JSON_ARRAY) {
			array::iterator it;
			const char* getKey = searchFor.begin().key().c_str();
			const value getVal = searchFor[getKey];
			int iGetType = searchFor[getKey].myType;
			size_t iIndex = 0;
			for (it = arr->begin(); it != arr->end(); ++it) {
				if (searchFor.myType == JSON_OBJECT && it->myType == JSON_OBJECT) {
					if (bSubStr && ((*it)[getKey].myType == JSON_STRING && iGetType == JSON_STRING)) {
						if ((*it)[getKey].str.find(getVal.str) != sdstring::npos) {
							retVal[iIndex++] = (*it);
						}
					} else {
						if ((*it)[getKey] == getVal) {
							retVal[iIndex++] = (*it);
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != sdstring::npos) {
							retVal[iIndex++] = (*it);
						}
					} else {
						if (str == searchFor.str) {
							retVal[iIndex++] = (*it);
						}
					}
				} else if (searchFor.myType == it->myType) {
					if (searchFor == (*it)) {
						retVal[iIndex++] = (*it);
					}
				}
			}
		} else if (myType == JSON_OBJECT) {
			object::iterator it;
			const char* getKey = searchFor.begin().key().c_str();
			const value getVal = searchFor[getKey];
			int iGetType = searchFor[getKey].myType;
			for (it = obj->begin(); it != obj->end(); ++it) {
				if (searchFor.myType == JSON_OBJECT && it->second.myType == JSON_OBJECT) {
					if (bSubStr && ((it->second)[getKey].myType == JSON_STRING && iGetType == JSON_STRING)) {
						if ((it->second)[getKey].str.find(getVal.str) != sdstring::npos) {
							retVal[it->first] = it->second;
						}
					} else {
						if ((it->second)[getKey] == getVal) {
							retVal[it->first] = it->second;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->second.myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != sdstring::npos) {
							retVal[it->first] = it->second;
						}
					} else {
						if (str == searchFor.str) {
							retVal[it->first] = it->second;
						}
					}
				} else if (searchFor.myType == it->second.myType) {
					if (searchFor == it->second) {
						retVal[it->first] = it->second;
					}
				}
			}
		}
		return retVal;
	}

	size_t value::simpleCount(value& searchFor, bool bSubStr) {
		size_t retVal = 0;

		if (myType == JSON_ARRAY) {
			array::iterator it;
			for (it = arr->begin(); it != arr->end(); ++it) {
				if (searchFor.myType == JSON_OBJECT && it->myType == JSON_OBJECT) {
					const char * key = searchFor.begin().key().c_str();
					if (bSubStr && ((*it)[key].myType == JSON_STRING && searchFor[key].myType == JSON_STRING)) {
						if ((*it)[key].str.find(searchFor[key].str) != sdstring::npos) {
							retVal++;
						}
					} else {
						if ((*it)[key] == searchFor[key]) {
							retVal++;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != sdstring::npos) {
							retVal++;
						}
					} else {
						if (str == searchFor.str) {
							retVal++;
						}
					}
				} else if (searchFor.myType == it->myType) {
					if (searchFor == (*it)) {
						retVal++;
					}
				}
			}
		} else if (myType == JSON_OBJECT) {
			object::iterator it;
			for (it = obj->begin(); it != obj->end(); ++it) {
				if (searchFor.myType == JSON_OBJECT && it->second.myType == JSON_OBJECT) {
					const char * key = searchFor.begin().key().c_str();
					if (bSubStr && ((it->second)[key].myType == JSON_STRING && searchFor[key].myType == JSON_STRING)) {
						if ((it->second)[key].str.find(searchFor[key].str) != sdstring::npos) {
							retVal++;
						}
					} else {
						if ((it->second)[key] == searchFor[key]) {
							retVal++;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->second.myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != sdstring::npos) {
							retVal++;
						}
					} else {
						if (str == searchFor.str) {
							retVal++;
						}
					}
				} else if (searchFor.myType == it->second.myType) {
					if (searchFor == it->second) {
						retVal++;
					}
				}
			}
		}
		return retVal;
	}

	void value::clear() {
		if (myType == JSON_ARRAY) {
			delete arr;
			arr = new array();
			if (pParentObject) {
				arr->setParentObject(pParentObject);
			} else if (pParentArray) {
				arr->setParentArray(pParentArray);
			}
		}
		if (myType == JSON_OBJECT) {
			delete obj;
			obj = new object();
			if (pParentObject) {
				obj->setParentObject(pParentObject);
			} else if (pParentArray) {
				obj->setParentArray(pParentArray);
			}
		}
		if (!str.empty())
			str.clear();

		m_number = 0;
		m_places = -1;
		m_boolean = false;
	}


	void value::destroy() {
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		str.clear();
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		myType = JSON_VOID;
	}

	void value::sort(bool (*compareFunc)(value&, value&)) {
		if (myType == JSON_ARRAY) {
			DEBUGPTR oldDebug = debug;
			debug = NULL;
			std::sort(arr->begin(), arr->end(), compareFunc);
			debug = oldDebug;
		}
	}

	double value::number() const {
		switch (myType) {
		case JSON_NUMBER:
			return m_number;

		case JSON_BOOLEAN:
			return m_boolean ? 1.0 : 0.0;

		case JSON_STRING: {
			if (str.empty()) {
				return 0.0;
			}
			std::istringstream convert(str.c_str());
			double d = 0.0;
			if (!(convert >> d))
				return 0.0;
			return d;
		}
		case JSON_OBJECT:
		{
			iterator it = (*this).find("#value");
			if (it != (*this).end()) {
				return (*it).m_number;
			}/* else {
				str.erase();
			}*/
			return 0.0;
		}
		default:
			return 0.0;
		}
	}

	i64 value::integer() const {
		return (i64)number();
	}

	ui64 value::_uint64() const {
		return (ui64)number();
	}

	float value::_float() const {
		return (float)number();
	}

	long value::_long() const {
		return (long)number();
	}

	double value::_double() const {
		return number();
	}

	int value::_int() const {
		return (int)number();
	}

	i64 value::_integer64() const {
		return (i64)number();
	}

	size_t value::_size_t() const {
		return (size_t)number();
	}

	short value::_short() const {
		return (short)number();
	}

	char value::_char() const {
		return (char)number();
	}

	unsigned long value::_ulong() const {
		return (unsigned long)number();
	}

	unsigned int value::_uint() const {
		return (unsigned int)number();
	}

	unsigned short value::_ushort() const {
		return (unsigned short)number();
	}

	unsigned char value::_uchar() const {
		return (unsigned char)number();
	}

    sdstring& value::_sdstring() {
		switch (myType) {
		case JSON_STRING:
			break;

		case JSON_NUMBER: {
			if (str.empty()) {
				makeStringFromNumber(str, m_places, m_number);
			}
			break;
		}

		case JSON_BOOLEAN:
			if (m_boolean)
				str.assign("true");
			else
				str.assign("false");
			break;

		case JSON_OBJECT:
		{
			iterator it = (*this).find("#value");
			if (it != (*this).end()) {
				str.assign((*it).str);
			} else {
				str.erase();
			}
			break;
		}

		default:
			if (!str.empty()) {
				str.erase();
			}
			break;
		}
        return str;
	}
    
    std::string& value::string() {
        return static_cast<std::string&>(_sdstring());
    }

//	std::string& value::string()
//	{
//		return reinterpret_cast<std::string&>(_sdstring());
//	}

	sdstring& value::stringC(sdstring & dest) const {
		switch (myType) {
		case JSON_STRING:
			dest.assign(str);
			break;

		case JSON_NUMBER: {
			// if (str.empty()) {
				makeStringFromNumber(dest, m_places, m_number);
			// }
			break;
		}

		case JSON_BOOLEAN:
			if (m_boolean)
				dest = "true";
			else
				dest = "false";
			break;

		case JSON_OBJECT:
		{
			iterator it = (*this).find("#value");
			if (it != (*this).end()) {
				dest.assign((*it).str);
			} else {
				dest.erase();
			}
			break;
		}

		default:
			dest.erase();
			break;
		}
		return dest;
	}

	const char* value::safeCString() {
		return _sdstring().c_str();
	}

	const char* value::c_str() {
		return _sdstring().c_str();
	}

	const char* value::cString() {
		switch (myType) {
		case JSON_VOID:
		case JSON_NULL:
			return NULL;
		default:
			return _sdstring().c_str();
		}
	}

	sdstring value::print(size_t depth, bool bPretty) const
	{
		if (myType == JSON_ARRAY) {
			arr->resize(size());
		}
		size_t l = psize(depth, bPretty);
		sdstring ret;
		MovingCharPointer ptr(ret, l);
		cprint(ptr, depth, bPretty);
		return ret;
	}

	bool value::operator==(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return V.isA(JSON_VOID) || V.isA(JSON_NULL) || V.boolean() == false;

			case JSON_BOOLEAN:
				return m_boolean == V.boolean();

			case JSON_NUMBER:
				return std::equal_to<double>()(m_number, V.number());

			case JSON_STRING:
			{
				sdstring temp;
				return str == V.stringC(temp);
			}

			case JSON_ARRAY:
			{
				if (V.myType != JSON_ARRAY)
					return false;
				return *arr == *(V.arr);
			}
			case JSON_OBJECT:
			{
				if (V.myType != JSON_OBJECT)
					return false;
				return *obj == *(V.obj);
			}
		}
	}

	bool value::operator!=(const value& V) const
	{
		return !(*this == V);
	}

	bool value::operator>(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return false;

			case JSON_BOOLEAN:
				return m_boolean != V.boolean();

			case JSON_NUMBER:
				return m_number > V.number();

			case JSON_STRING:
			{
				sdstring temp;
				return str > V.stringC(temp);
			}

			case JSON_ARRAY:
				if (V.myType == JSON_ARRAY)
					return *arr > *(V.arr);
				else
					return true;

			case JSON_OBJECT:
				if (V.myType == JSON_OBJECT)
					return *obj > *(V.obj);
				else
					return true;
		}
	}

	bool value::operator<(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				if (V.myType > JSON_NULL) {
					return true;
				} else {
					return false;
				}

			case JSON_BOOLEAN:
				return m_boolean != V.boolean();

			case JSON_NUMBER:
				return m_number < V.number();

			case JSON_STRING:
			{
				sdstring temp;
				return str < V.stringC(temp);
			}

			case JSON_ARRAY:
				if (V.myType == JSON_ARRAY)
					return *arr < *(V.arr);
				else
					return false;

			case JSON_OBJECT:
				if (V.myType == JSON_OBJECT)
					return *obj < *(V.obj);
				else
					return false;
		}
	}

	bool value::operator<=(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return true;

			case JSON_BOOLEAN:
				return true;

			case JSON_NUMBER:
				return m_number <= V.number();

			case JSON_STRING:
			{
				sdstring temp;
				return str <= V.stringC(temp);
			}

			case JSON_ARRAY:
				if (V.myType == JSON_ARRAY)
					return *arr <= *(V.arr);
				else
					return false;

			case JSON_OBJECT:
				if (V.myType == JSON_OBJECT)
					return *obj <= *(V.obj);
				else
					return false;
		}
	}

	bool value::operator>=(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				if (V.myType > JSON_NULL) {
					return false;
				} else {
					return true;
				}

			case JSON_BOOLEAN:
				return true;

			case JSON_NUMBER:
				return m_number >= V.number();

			case JSON_STRING:
			{
				sdstring temp;
				return str >= V.stringC(temp);
			}

			case JSON_ARRAY:
				if (V.myType == JSON_ARRAY)
					return *arr >= *(V.arr);
				else
					return true;

			case JSON_OBJECT:
				if (V.myType == JSON_OBJECT)
					return *obj >= *(V.obj);
				else
					return true;
		}
	}

	value value::operator+(const value& V) const
	{
		switch (myType) {
			default:
				return value(0);

			case JSON_VOID:
			case JSON_NULL:
				return value(V.number());

			case JSON_BOOLEAN:
				return value(m_boolean + V.boolean());

			case JSON_NUMBER:
				return value(m_number + V.number());

			case JSON_STRING:
			{
				sdstring temp;
				return value(str + V.stringC(temp));
			}

			case JSON_ARRAY:
			case JSON_OBJECT:
			{
				value ret = *this;
				ret.insert(ret.end(), V.begin(), V.end());
				if (ret.myType == JSON_OBJECT && ret.obj) {
					ret.obj->setNotEmpty();
				} else if (ret.myType == JSON_ARRAY && ret.arr) {
					ret.arr->setNotEmpty();
				}
				return ret;
			}
		}
	}

	value value::operator-(const value& V) const
	{
		switch (myType) {
			default:
				return value(0);

			case JSON_VOID:
			case JSON_NULL:
				return value(V.number());

			case JSON_BOOLEAN:
				return value(m_boolean - V.boolean());

			case JSON_NUMBER:
				return value(m_number - V.number());

			case JSON_OBJECT:
			{
				value ret(*this);
				for (iterator it = V.begin(); it != V.end(); ++it) {
					ret.erase(it.key().string());
				}
				return ret;
			}
		}
	}

	value value::operator*(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return value(0);

			case JSON_BOOLEAN:
				return value(m_boolean * V.boolean());

			case JSON_NUMBER:
				return value(m_number * V.number());
		}
	}

	value value::operator/(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return value(0);

			case JSON_NUMBER:
				return value(m_number / V.number());
		}
	}

	value value::operator%(const value& V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return value(0);

			case JSON_NUMBER:
				return value((i64)m_number % V.integer());
		}
	}

	value& value::operator+=(const value& V)
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = V;
				break;

			case JSON_NUMBER:
				m_number += V.number();
				str.clear();
				break;

			case JSON_STRING:
			{
				sdstring temp;
				str += V.stringC(temp);
				break;
			}

			case JSON_ARRAY:
				insert(end(), V.begin(), V.end());
				arr->setNotEmpty();
				break;

			case JSON_OBJECT:
				insert(end(), V.begin(), V.end());
				obj->setNotEmpty();
				break;

			default:
				break;
		}
		return *this;
	}

	value& value::operator-=(const value& V)
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = -V;
				break;

			case JSON_NUMBER:
				m_number -= V.number();
				str.clear();
				break;

			case JSON_OBJECT:
				for (iterator it = V.begin(); it != V.end(); ++it) {
					erase(it.key().string());
				}
				break;

			default:
				break;
		}
		return *this;
	}


	value &value::operator*=(const value& V)
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = 0;
				break;

			case JSON_NUMBER:
				m_number *= V.number();
				str.clear();
				break;

			default:
				break;
		}
		return *this;
	}

	value &value::operator/=(const value& V)
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = 0;
				break;

			case JSON_NUMBER:
				m_number /= V.number();
				str.clear();
				break;

			default:
				break;
		}
		return *this;
	}

	value &value::operator%=(const value& V)
	{
		switch (myType) {
			case JSON_NUMBER:
				m_number = (double)((i64)m_number % V.integer());
				str.clear();
				break;

			default:
				break;
		}
		return *this;
	}

	value &value::operator++()
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = 1;
				break;

			case JSON_BOOLEAN:
				m_boolean =! m_boolean;
				break;

			case JSON_NUMBER:
				m_number++;
				str.clear();
				break;

			default:
				break;
		}
		return *this;
	}

	value &value::operator--()
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = -1;
				break;

			case JSON_BOOLEAN:
				m_boolean =! m_boolean;
				break;

			case JSON_NUMBER:
				m_number--;
				str.clear();
				break;

			default:
				break;
		}
		return *this;
	}

	value value::operator++(int)
	{
		value ret(*this);
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = 1;
				ret = 0;
				break;

			case JSON_BOOLEAN:
				m_boolean =! m_boolean;
				break;

			case JSON_NUMBER:
				m_number++;
				str.clear();
				break;

			default:
				break;
		}
		return ret;
	}

	value value::operator--(int)
	{
		value ret(*this);
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
				*this = -1;
				ret = 0;
				break;

			case JSON_BOOLEAN:
				m_boolean =! m_boolean;
				break;

			case JSON_NUMBER:
				m_number--;
				str.clear();
				break;

			default:
				break;
		}
		return ret;
	}

	value value::operator-() const
	{
		if (myType == JSON_NUMBER) {
			return value(-number());
		}
		return *this;
	}

	std::ostream& operator<<(std::ostream& S, document& doc) {
		S << doc.write(true).c_str();
		return S;
	}

	std::ostream& operator<<(std::ostream& S, value& doc) {
		switch (doc.isA()) {
		case JSON_VOID:
		case JSON_NULL:
		case JSON_ARRAY:
		case JSON_OBJECT:
		default:
			break;

		case JSON_BOOLEAN:
			if (doc.boolean())
				S << "true";
			else
				S << "false";
			break;

		case JSON_NUMBER:
			S << doc.string();
			break;

		case JSON_STRING: {
			S << doc.string();
			break;
		}
		}
		return S;
	}

	document::document(const document& V) : value((const value &)V), strParseResult(V.strParseResult)
	{
		bParseSuccessful = V.bParseSuccessful;
		// strParseResult = V.strParseResult;
		// m_number = V.m_number;
		// m_boolean = V.m_boolean;

		// if (!V.str.empty()) {
		// 	str.assign(V.str);
		// }

		// myType = V.myType;

		// obj = NULL;
		// if (V.obj) {
		// 	obj = new object(V.obj);
		// }

		// arr = NULL;
		// if (V.arr) {
		// 	arr = new array(V.arr);
		// }
	}

	document::document(document&& V) : value((const value &)V), strParseResult(std::move(V.strParseResult))
	{
		std::swap(bParseSuccessful, V.bParseSuccessful);
		std::swap(strParseResult, V.strParseResult);
		// m_number = V.m_number;
		// m_boolean = V.m_boolean;

		// if (!V.str.empty()) {
		// 	str.assign(V.str);
		// }

		// myType = V.myType;

		// obj = NULL;
		// if (V.obj) {
		// 	obj = new object(V.obj);
		// }

		// arr = NULL;
		// if (V.arr) {
		// 	arr = new array(V.arr);
		// }
	}

	document& document::operator=(const document& V)
	{
		if (this == &V)
			return *this;

		if (debug) {
			if (myType != V.myType) {
				switch(myType) {
						// case JSON_NULL:
						// 	debug("json operator= changed type from NULL to %s", typeName(V.myType).c_str());
						// 	break;

					case JSON_BOOLEAN:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Boolean %i to %s", m_boolean, typeName(V.myType));
						}
						break;

					case JSON_NUMBER:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Number %f to %s", m_number, typeName(V.myType));
						}
						break;

					case JSON_STRING:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from String '%s' to %s", str.c_str(), typeName(V.myType));
						}
						break;

					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(V.myType), ((value*)this)->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(V.myType), ((value*)this)->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}

		m_number = V.m_number;
		m_places = V.m_places;
		m_boolean = V.m_boolean;

		str.assign(V.str);



		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}

		myType = V.myType;


		if (V.myType == JSON_OBJECT) {
			obj = new object(V.obj);
		} else if (V.myType == JSON_ARRAY) {
			arr = new array(V.arr);
		}

		if (myType != JSON_VOID) {
			if (pParentObject) {
				pParentObject->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentObject(pParentObject);
				} else if (myType == JSON_OBJECT) {
					obj->setParentObject(pParentObject);
				}
			} else if (pParentArray) {
				pParentArray->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentArray(pParentArray);
				} else if (myType == JSON_OBJECT) {
					obj->setParentArray(pParentArray);
				}
			}
		}

		bParseSuccessful = V.bParseSuccessful;
		strParseResult = V.strParseResult;

		return *this;
	}

	document& document::operator=(document&& V)
	{

		if (debug) {
			if (myType != V.myType) {
				switch(myType) {
						// case JSON_NULL:
						// 	debug("json operator= changed type from NULL to %s", typeName(V.myType).c_str());
						// 	break;

					case JSON_BOOLEAN:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Boolean %i to %s", m_boolean, typeName(V.myType));
						}
						break;

					case JSON_NUMBER:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from Number %f to %s", m_number, typeName(V.myType));
						}
						break;

					case JSON_STRING:
						if (V.myType == JSON_OBJECT || V.myType == JSON_ARRAY) {
							debug("json operator= changed type from String '%s' to %s", str.c_str(), typeName(V.myType));
						}
						break;

					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(V.myType), ((value*)this)->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(V.myType), ((value*)this)->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}

		m_number = std::move(V.m_number);
		m_places = std::move(V.m_places);
		m_boolean = std::move(V.m_boolean);

		str = std::move(V.str);

		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;
			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		myType = std::move(V.myType);

		obj = std::move(V.obj);

		if (myType != JSON_VOID) {
			if (pParentObject) {
				pParentObject->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentObject(pParentObject);
				} else if (myType == JSON_OBJECT) {
					obj->setParentObject(pParentObject);
				}
			} else if (pParentArray) {
				pParentArray->setNotEmpty();
				if (myType == JSON_ARRAY) {
					arr->setParentArray(pParentArray);
				} else if (myType == JSON_OBJECT) {
					obj->setParentArray(pParentArray);
				}
			}
		}

		// V.m_number = 0;
		// V.m_places = -1;
		// V.m_boolean = false;
		V.myType = JSON_VOID;

		// V.obj = NULL;
		// V.arr = NULL;

		V.pParentObject = NULL;
		V.pParentArray = NULL;

		std::swap(bParseSuccessful, V.bParseSuccessful);
		std::swap(strParseResult, V.strParseResult);
		return *this;

	}

	bool document::parse(const sdstring& inStr, PREPARSEPTR preParser, const sdstring& preParseFileName) {
		return parse(inStr.c_str(), inStr.size(), preParser, preParseFileName);
	}

	bool document::parse(const char* inDat, size_t len, PREPARSEPTR preParser, const sdstring &preParseFileName) {
		strParseResult = "Successful";
		bool bFailed = false;
		bParseSuccessful = true;
		switch (myType) {
			case JSON_OBJECT:
				delete obj;
				obj = NULL;
				break;

			case JSON_ARRAY:
				delete arr;
				arr = NULL;
				break;

			default:
				break;
		}
		myType = JSON_VOID;
		m_number = 0;
		m_places = -1;
		m_boolean = false;
		str.clear();
		sdstring sOut;
		const char * inStr = inDat;
		if (preParser != NULL) {
			sdstring sDat(inDat, len);
			preParser(sDat, sOut, preParseFileName);
			if (sOut.size() == 0) {
				bParseSuccessful = false;
				bFailed = true;
				strParseResult = "JSON Document failed to pre-parse.";
				if (debug) {
					debug("%s", strParseResult.c_str());
					debug("%s", inStr);
				}
				return false;
			}
			inStr = sOut.c_str();
		}
		instring in(inStr);
		SkipWhitespace(in);
		if (in.tell() >= in.size()) {
			return true;
		}
		valueParse(*this, in, &bFailed);
		if (bFailed) {
			strParseResult = in.Str();
			bParseSuccessful = false;
			if (debug) {
				debug("%s", strParseResult.c_str());
				debug("%s", inStr);
			}
		}
		return !bFailed;
	}

	bool document::parseFile(const sdstring &inStr, PREPARSEPTR preParser, bool bReWriteFile) {
		FILE* fd = fopen(inStr.c_str(), "rb");
#if defined _JSON_TEMP_FILES_ && defined _JSON_RESTORE_TEMP_FILES_
		sdstring sInstrPlusBak(inStr);
		sInstrPlusBak.append(".bak");

		if (fd == NULL) {
			if (fileExists(sInstrPlusBak.c_str())) {
				fd = fopen(sInstrPlusBak.c_str(), "rb");
				if (debug && fd) {
					debug("File opened from backup %s.", sInstrPlusBak.c_str());
				}
			}
		} else {
			if (fd && fileExists(sInstrPlusBak.c_str())) {
				if (remove(sInstrPlusBak.c_str()) != 0 && debug) {
					debug("Failed remove backup of %s.", inStr.c_str());
				}
			}
		}
#endif
		if (fd) {
			fseek(fd, 0, SEEK_END);
			size_t l = (size_t)ftell(fd);
			fseek(fd, 0, SEEK_SET);
			char* buffer = static_cast<char*>(malloc(l + 1));

			buffer[l] = 0;
			size_t br = fread(buffer, 1, l, fd);
			if (debug && br != l) {
				debug("File size mismatch in %s.", inStr.c_str());
			}

			fclose(fd);
			bool bRetVal;
			if (bReWriteFile) {
				bRetVal = parse(buffer, l, preParser, inStr);
			} else {
				bRetVal = parse(buffer, l, preParser);
			}
			bParseSuccessful = bRetVal;
			if (debug && !bParseSuccessful) {
				debug("JSON could not parse %s.", inStr.c_str());
			}
			memset(buffer, 0, l+1);
			free(buffer);
			return bRetVal;
		}

		bParseSuccessful = false;
		return false;
	}

	sdstring document::write(bool bPretty, PREWRITEPTR preWriter) const
	{
		return write(1, bPretty, preWriter);
	}

	sdstring document::write(size_t iDepth, bool bPretty, PREWRITEPTR preWriter) const
	{
		if (isA(JSON_VOID)) {
			return "";
		} else if (isA(JSON_OBJECT)) {
			size_t l = obj->psize(iDepth, bPretty);
			sdstring ret;
			MovingCharPointer ptr(ret, l);
			obj->cprint(ptr, iDepth, bPretty);
			if (preWriter == NULL) {
				return ret;
			} else {
				sdstring sOut;
				return preWriter(ret, sOut);
			}
		} else if (isA(JSON_ARRAY)) {
			arr->resize(size());
			size_t l = arr->psize(iDepth, bPretty);
			sdstring ret;
			MovingCharPointer ptr(ret, l);
			arr->cprint(ptr, iDepth, bPretty);

			if (preWriter == NULL) {
				return ret;
			} else {
				sdstring sOut;
				return preWriter(ret, sOut);
			}
		} else {
			size_t l = value::psize(iDepth, bPretty);
			sdstring t;
			MovingCharPointer ptr(t, l);
			value::cprint(ptr, iDepth, bPretty);
			if (preWriter == NULL) {
				return t;
			} else {
				sdstring sDat(t);
				sdstring sOut;
				return preWriter(sDat, sOut);
			}
		}
	}

	bool fileExists(const char * szName)
	{
		if (szName == NULL) {
			return false;
		}
		struct stat buffer;
		return (stat(szName, &buffer) == 0);
	}

	bool document::writeFile(const sdstring& inStr, bool bPretty, PREWRITEPTR preWriter) const
	{
#if defined _JSON_TEMP_FILES_
		sdstring sTempFile(inStr);
		sTempFile.append(".tmp");

		FILE* fd = fopen(sTempFile.c_str(), "wb");
		if (fd) {
			sdstring w = write(bPretty, preWriter);

			if (fwrite(w.data(), 1, w.size(), fd) != w.size()) {
				if (debug) {
					debug("Failed Writing to %s.", inStr.c_str());
				}
				fclose(fd);
				return false;
			} else {
				sdstring sInstrPlusBak(inStr);
				sInstrPlusBak.append(".bak");
				fclose(fd);
				if (fileExists(sInstrPlusBak.c_str())) {
					remove(sInstrPlusBak.c_str());
				}
				if (fileExists(inStr.c_str())) {
					if (rename(inStr.c_str(), sInstrPlusBak.c_str()) != 0) {
						if (debug) {
							debug("Failed to backup %s.", inStr.c_str());
						}
						return false;
					}
				}
				if (rename(sTempFile.c_str(), inStr.c_str()) != 0) {
					if (debug) {
						debug("Failed rename temp file to %s.", inStr.c_str());
					}
					if (rename(sInstrPlusBak.c_str(), inStr.c_str()) != 0 && debug) {
						debug("Failed restore backup of %s.", inStr.c_str());
					}
					return false;
				}

				if (fileExists(sInstrPlusBak.c_str())) {
					if (remove(sInstrPlusBak.c_str()) != 0 && debug) {
						debug("Failed remove backup of %s.", inStr.c_str());
					}
				}

				return true;
			}
		}
#else
		FILE* fd = fopen(inStr.c_str(), "wb");
		if (fd) {
			sdstring w(write(bPretty, preWriter));
			fwrite(w.data(), 1, w.size(), fd);
			fclose(fd);
			return true;
		}
#endif
		return false;
	}

	int document::appendToArrayFile(const sdstring& sFile, const document & atm, bool bPretty)
	{
		FILE* fd = fopen(sFile.c_str(), "r+b");
		// int iError = 0;
		if (!fd) {
			fd = fopen(sFile.c_str(), "wb");
			if (fd) {
				fputc('[', fd);
				if (bPretty) {
					fputc('\n', fd);
					fputc('\t', fd);
				}
			}
		} else {
			int cFirst = fgetc(fd);
			if (cFirst == '[') {
				fseek(fd, -1, SEEK_END);
				// iError = ferror(fd);
				while(fgetc(fd) != ']') {
					fseek(fd, -2, SEEK_CUR);
					// iError = ferror(fd);
				};
				int c;
				do{
					fseek(fd, -2, SEEK_CUR);
					// iError = ferror(fd);
					c = fgetc(fd);
					// iError = ferror(fd);
				} while (c == '\r' || c == '\n' || c == '\t');
				fseek(fd, 0, SEEK_CUR);
				if (c != '[') {
					fputc(',', fd);
					// iError = ferror(fd);
				}
				if (bPretty) {
					fputc('\n', fd);
					// iError = ferror(fd);
					fputc('\t', fd);
					// iError = ferror(fd);
				}
			} else if (cFirst == EOF) {
				fputc('[', fd);
				// iError = ferror(fd);
				if (bPretty) {
					fputc('\n', fd);
					// iError = ferror(fd);
					fputc('\t', fd);
					// iError = ferror(fd);
				}
			} else {
				fclose(fd);
				// iError = ferror(fd);
				return -1;
			}
		}
		if (fd) {
			sdstring sNew(atm.write(2, bPretty).c_str());
			fwrite(sNew.data(), 1, sNew.size(), fd);
			// iError = ferror(fd);
			if (bPretty) {
				fputc('\n', fd);
				// iError = ferror(fd);
			}
			fputc(']', fd);
			// iError = ferror(fd);
			int iPos = (int)ftell(fd);
			// iError = ferror(fd);
			fclose(fd);
			return iPos;
		}
		return -1;
	}

	iterator::iterator(iterator&& it)// : arr_it(std::move(it.arr_it)), obj_it(std::move(it.obj_it)), bNone(std::move(it.bNone)), bIsArray(std::move(it.bIsArray)), bSetKey(std::move(it.bSetKey)) 
	{
		std::swap(bNone, it.bNone);
		std::swap(arr_it, it.arr_it);
		std::swap(obj_it, it.obj_it);
		std::swap(bIsArray, it.bIsArray);
		bSetKey = false;
		it.bSetKey = false;
	}

	iterator& iterator::operator=(const iterator& it) 
	{
		if (this == &it) {
			return *this;
		}
		dumbRet = it.dumbRet;
		bNone = it.bNone;
		arr_it = it.arr_it;
		obj_it = it.obj_it;
		bIsArray = it.bIsArray;
		bSetKey = false;
		return *this;
	}
	
	iterator& iterator::operator=(iterator&& it) 
	{
		std::swap(dumbRet, it.dumbRet);
		std::swap(bNone, it.bNone);
		std::swap(arr_it, it.arr_it);
		std::swap(obj_it, it.obj_it);
		std::swap(bIsArray, it.bIsArray);
		bSetKey = false;
		it.bSetKey = false;
		return *this;
	}

	iterator::~iterator() 
	{
		if (bSetKey) {
			obj_it->second.m_key.clear();
		}
	}

	iterator& iterator::operator++() 
	{
		if (!bNone) {
			if (bIsArray) {
				++arr_it;
			} else {
				if (bSetKey) {
					obj_it->second.m_key.clear();
					bSetKey = false;
				}
				++obj_it;
			}
		}
		return *this;
	}

	iterator iterator::operator++(int) 
	{
		iterator tmp(*this);
		operator++();
		return tmp;
	}

	iterator& iterator::operator--() 
	{
		if (!bNone) {
			if (bIsArray) {
				--arr_it;
			} else {
				if (bSetKey) {
					obj_it->second.m_key.clear();
					bSetKey = false;
				}
				--obj_it;
			}
		}
		return *this;
	}

	iterator iterator::operator--(int) 
	{
		iterator tmp(*this);
		operator--();
		return tmp;
	}
#if defined _WIN32
	bool iterator::operator==(iterator const& rhs) const
	{
		if (bNone && rhs.bNone)
			return true;
		if (bIsArray) {
			return arr_it == rhs.arr_it;
		} else {
			return obj_it == rhs.obj_it;
		}
	}

	bool iterator::operator!=(iterator const& rhs) const
	{
		if (bNone && rhs.bNone)
			return false;
		if (bIsArray) {
			return arr_it != rhs.arr_it;
		} else {
			return obj_it != rhs.obj_it;
		}
	}
#else
	bool iterator::operator==(iterator const& rhs)
	{
		if (bNone && rhs.bNone)
			return true;
		if (bIsArray) {
			return arr_it == rhs.arr_it;
		} else {
			return obj_it == rhs.obj_it;
		}
	}

	bool iterator::operator!=(iterator const& rhs)
	{
		if (bNone && rhs.bNone)
			return false;
		if (bIsArray) {
			return arr_it != rhs.arr_it;
		} else {
			return obj_it != rhs.obj_it;
		}
	}
#endif
	value& iterator::operator*() 
	{
		if (!bNone) {
			if (bIsArray) {
				return *arr_it;
			} else {
				if (!bSetKey) {
					obj_it->second.m_key.assign(obj_it->first);
					bSetKey = true;
				}
				return obj_it->second;
			}
		} else {
			dumbRet = value();
			return dumbRet;
		}
	}

	value iterator::key() 
	{
		if (!bNone) {
			if (bIsArray)
				return value();
			else
				return value(obj_it->first);
		} else {
			return value();
		}
	}

		reverse_iterator::reverse_iterator(reverse_iterator&& it)// : arr_it(it.arr_it), obj_it(it.obj_it), dumbRet() 
		{
			std::swap(bNone, it.bNone);
			std::swap(arr_it, it.arr_it);
			std::swap(obj_it, it.obj_it);
			std::swap(bIsArray, it.bIsArray);
			bSetKey = false;
			it.bSetKey = false;
		}

		reverse_iterator & reverse_iterator::operator=(const reverse_iterator& it)
		{
			if (this == &it) {
				return *this;
			}
			dumbRet = it.dumbRet;
			bNone = it.bNone;
			arr_it = it.arr_it;
			obj_it = it.obj_it;
			bIsArray = it.bIsArray;
			bSetKey = false;
			// it.bSetKey = false;
			return *this;
		}

		reverse_iterator & reverse_iterator::operator=(reverse_iterator&& it)
		{
			std::swap(dumbRet, it.dumbRet);
			std::swap(bNone, it.bNone);
			std::swap(arr_it, it.arr_it);
			std::swap(obj_it, it.obj_it);
			std::swap(bIsArray, it.bIsArray);
			bSetKey = false;
			it.bSetKey = false;
			return *this;
		}

		reverse_iterator::~reverse_iterator()
		{
			if (bSetKey) {
				obj_it->second.m_key.clear();
			}
		}

		reverse_iterator& reverse_iterator::operator++()
		{
			if (!bNone) {
				if (bIsArray) {
					++arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					++obj_it;
				}
			}
			return *this;
		}

		reverse_iterator reverse_iterator::operator++(int)
		{
			reverse_iterator tmp(*this);
			operator++();
			return tmp;
		}

		reverse_iterator& reverse_iterator::operator--()
		{
			if (!bNone) {
				if (bIsArray) {
					--arr_it;
				} else {
					if (bSetKey) {
						obj_it->second.m_key.clear();
						bSetKey = false;
					}
					--obj_it;
				}
			}
			return *this;
		}

		reverse_iterator reverse_iterator::operator--(int)
		{
			reverse_iterator tmp(*this);
			operator--();
			return tmp;
		}

		bool reverse_iterator::operator==(const reverse_iterator& rhs) const
		{
			if (bNone && rhs.bNone)
				return true;
			if (bIsArray) {
				return arr_it == rhs.arr_it;
			} else {
				return obj_it == rhs.obj_it;
			}
		}

		bool reverse_iterator::operator!=(const reverse_iterator& rhs) const
		{
			if (bNone && rhs.bNone)
				return false;
			if (bIsArray) {
				return arr_it != rhs.arr_it;
			} else {
				return obj_it != rhs.obj_it;
			}
		}

		value& reverse_iterator::operator*()
		{
			if (!bNone) {
				if (bIsArray) {
					return *arr_it;
				} else {
					if (!bSetKey) {
						(obj_it->second).m_key.assign(obj_it->first);
						bSetKey = true;
					}
					return obj_it->second;
				}
			} else {
				dumbRet = value();
				return dumbRet;
			}
		}

		value reverse_iterator::key()
		{
			if (!bNone) {
				if (bIsArray)
					return value();
				else
					return value(obj_it->first);
			} else {
				return value();
			}
		}

}


#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn + 8026
#pragma warn + 8027
#endif
