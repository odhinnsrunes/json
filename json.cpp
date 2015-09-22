/*
Copyright (c) 2012-2015 James Baker

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


#include "json.hpp"
#include <assert.h>
#include <algorithm>
#include <iomanip>
#include <stdarg.h>
#if defined _WIN32 && defined __clang__
#define __uncaught_exception std::uncaught_exception
#endif
#include <thread>

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

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRICATE 1
#define _CRT_SECURE_NO_WARNINGS 1
#endif

void JSONDebug(const char * format, ...) {
	std::string s;
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

namespace json
{
	class MovingCharPointer
	{
	public:
		MovingCharPointer(size_t reserve);
		MovingCharPointer(const MovingCharPointer& ptr);
		~MovingCharPointer();

		void set(const std::string& str);
		void set(const char* n, size_t size);
		void set(char n);

		char* orig();

	private:
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
		std::string in = inputString.SoFar();
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
		instring ostring(s.str());
		inputString = ostring;
	}

	void nullParse(value& ret, instring& inputString, bool* bFailed) {
		if (inputString.take() != 'n' || inputString.take() != 'u' || inputString.take() != 'l' || inputString.take() != 'l') {
			generateError(inputString, "Error Parsing null.");
			*bFailed = true;
		}
		ret.myType = JSON_NULL;
		if (ret.obj)
			delete ret.obj;
		if (ret.arr)
			delete ret.arr;

		ret.str.clear();

		ret.m_number = 0.0;

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
			char c = s.take();
			ret <<= 4;
			ret += c;
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

	void stringParse(std::string& ret, instring& s, bool* bFailed) { 
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
			char c = s.take();
			switch (c) {
			case '\\': {
				char e = s.take();
				if (escape[(unsigned char)e])
					*ptr++ = escape[(unsigned char)e];
				else if (e == 'u') { // Unicode
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

			case '"': {
				ret.assign(retVal, ptr - retVal);
				return;
			}

			case 0: {
				generateError(s, "lacks ending quotation before the end of string");
				*bFailed = true;
				return;
			}
			default: {
				*ptr++ = c;
				break;
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
		ret.str = std::string(pStart, s.getPos() - pStart);
		size_t pos = 0;
		for(char c : ret.str){
			if(c != '0'){
				if(pos > 0){
					ret.str = ret.str.substr(pos);
				}
				if(ret.str[0] == '.'){
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
		if (ret.obj == NULL) {
			ret.obj = new object();
			if (ret.arr)
				delete ret.arr;
			ret.arr = NULL;
            ret.obj->setNotEmpty();
            if(ret.pParentArray){
                ret.obj->setParentArray(ret.pParentArray);
            } else if (ret.pParentObject) {
                ret.obj->setParentObject(ret.pParentObject);
            }
		}
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

			std::string key;
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
		if (arr.arr == NULL) {
			arr.arr = new array();
			if (arr.obj)
				delete arr.obj;
			arr.obj = NULL;
            if(arr.pParentArray){
                arr.arr->setParentArray(arr.pParentArray);
            } else if (arr.pParentObject) {
                arr.arr->setParentObject(arr.pParentObject);
            }
            arr.arr->setNotEmpty();
		}

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
			a.myType = JSON_STRING;
			if (a.obj)
				delete a.obj;
			if (a.arr)
				delete a.arr;
			a.m_boolean = false;
			a.m_number = 0.0;
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

	size_t esize(const std::string& in) {
		size_t ret = 0;
		static const char escape[256] = {
#define Z16 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
			//0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F
			6, 6, 6, 6, 6, 6, 6, 6, 2, 2, 2, 6, 2, 2, 6, 6, // 00
			6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, // 10
			1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,								// 20
			Z16, Z16,																		// 30~4F
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,								// 50
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16								// 60~FF
#undef Z16
		};
		size_t l = in.size();
		const char* str = in.c_str();
		for (size_t i = 0; i < l; i++) {
			ret += escape[(unsigned char)*(str++)];
		}
		return ret;
	}

	void escape(MovingCharPointer& ptr, const std::string& ins) 
	{
		static const char hexDigits[] = "0123456789ABCDEF";
		static const char escape[256] = {
#define Z16 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			//0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
			'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
			0, 0, '"', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,								// 20
			Z16, Z16,																		// 30~4F
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 0,								// 50
			Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16								// 60~FF
#undef Z16
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

	MovingCharPointer::MovingCharPointer(size_t reserve) {
		m_orig = new char[reserve + 1];
		m_orig[reserve] = 0;
		m_current = m_orig;
		m_max = m_orig + reserve;
	}

	MovingCharPointer::MovingCharPointer(const MovingCharPointer& ptr) {
		size_t reserve = ptr.m_max - ptr.m_orig;
		size_t offset = ptr.m_current - ptr.m_orig;
		m_orig = new char[reserve + 1];
		memcpy(m_orig, ptr.m_orig, reserve);
		m_orig[reserve] = 0;
		m_current = m_orig + offset;
		m_max = m_orig + reserve;
	}

	MovingCharPointer::~MovingCharPointer() {
		delete[] m_orig;
	}

	inline void MovingCharPointer::set(const std::string& str) {
		size_t l = str.size();
		memcpy(m_current, str.c_str(), l);
		m_current += l;
	}

	inline void MovingCharPointer::set(const char* n, size_t size) {
		memcpy(m_current, n, size);
		m_current += size;
	}

	inline void MovingCharPointer::set(char n) {
		*m_current++ = n;
	}

	char* MovingCharPointer::orig() {
		*m_current = 0;
		return m_orig;
	}

	instring::instring(const std::string& in) {
		pos = 0;
		m_size = in.size();
		str = new char[m_size + 1];
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
	}

	instring::instring(const instring& in) {
		pos = in.pos;
		m_size = in.m_size;
		str = new char[m_size + 1];
		memcpy(str, in.str, m_size);
		str[m_size] = 0;
	}

	instring::instring(char* in) {
		pos = 0;
		m_size = strlen(in);
		str = new char[m_size + 1];
		memcpy(str, in, m_size);
		str[m_size] = 0;
	}

	instring::~instring() {
		delete[] str;
	}

	inline char &instring::take() {
		return str[pos++];
	}
	
	inline void instring::skip() {
		pos++;
	}
	
	inline char &instring::peek() const
	{
		return str[pos];
	}

	inline size_t instring::tell() const
	{
		return pos;
	}

	inline size_t instring::size() const
	{
		return m_size;
	}

	void instring::seek(size_t newPos) {
		if (newPos < m_size) {
			pos = newPos;
		}
	}

	char* instring::getPos() {
		return &str[pos];
	}

	instring& instring::operator=(std::string& in) {
		pos = 0;
		m_size = in.size();
		delete[] str;
		str = new char[m_size + 1];
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
		return *this;
	}
	
	instring& instring::operator=(const char* in) {
		pos = 0;
		m_size = strlen(in);
		delete[] str;
		str = new char[m_size + 1];
		memcpy(str, in, m_size);
		str[m_size] = 0;
		return *this;
	}
	
	instring& instring::operator=(instring& in) {
		if (this == &in)
			return *this;
		pos = in.pos;
		m_size = in.m_size;
		delete[] str;
		str = new char[m_size + 1];
		memcpy(str, in.str, m_size);
		str[m_size] = 0;
		return *this;
	}
	
	void instring::set(std::string& in) {
		pos = 0;
		m_size = in.size();
		delete[] str;
		str = new char[m_size + 1];
		memcpy(str, in.c_str(), m_size);
		str[m_size] = 0;
	}
	
	void instring::set(const char* in) {
		pos = 0;
		m_size = strlen(in);
		delete[] str;
		str = new char[m_size + 1];
		memcpy(str, in, m_size);
		str[m_size] = 0;
	}
	
	instring instring::operator+(double V) const
	{
		std::string temp = *this;
		std::ostringstream o;
		o << std::setprecision(JSON_NUMBER_PRECISION) << V;
		temp.append(o.str());
		return temp;
	}

	instring instring::operator+(std::string& V) const
	{
		std::string temp = *this;
		temp.append(V);
		return temp;
	}

	instring instring::operator+(const char* V) const
	{
		std::string temp = *this;
		temp.append(V);
		return temp;
	}

	std::string instring::Str() const
	{
		return std::string(str);
	}

	std::string instring::SoFar() const
	{
		return std::string(str, pos);
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
				if (arr)
					return iterator(arr->end());
				else
					return iterator();
				
			case JSON_OBJECT:
				if (obj)
					return iterator(obj->end());
				else
					return iterator();

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
				if (arr)
					return reverse_iterator(arr->rend());
				else
					return reverse_iterator();
				
			case JSON_OBJECT:
				if (obj)
					return reverse_iterator(obj->rend());
				else
					return reverse_iterator();

			default:
				return reverse_iterator();
		}
	}

	iterator value::find(size_t index) const
	{
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json find: index %lu out of bounds", index);
			return iterator();
		}
		if (arr) {
			if (index < arr->size())
				return arr->begin() + index;
		}
		return iterator();
	}
	
	iterator value::find(std::string index) const 
	{
		if (obj) {
			return obj->find(index);
		}
		return iterator();
	}
	
	reverse_iterator value::rfind(size_t index) const {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json rfind: index %lu out of bounds", index);
			return reverse_iterator();
		}
		return reverse_iterator(find(index));
	}
	
	reverse_iterator value::rfind(std::string index) const {
		return reverse_iterator(find(index));
	}
	
	bool value::boolean() {
		switch (myType) {
		case JSON_VOID:
		case JSON_NULL:
		default:
			return false;
		case JSON_BOOLEAN:
			return m_boolean;
		case JSON_NUMBER:
			return m_number != 0;
		case JSON_STRING:
			return str.size() > 0;
		case JSON_ARRAY:
			return !arr->empty();
		case JSON_OBJECT:
			return !obj->empty();
		}
	}

	size_t value::psize(int depth, bool bPretty) const
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
				std::ostringstream s;
				s << std::setprecision(JSON_NUMBER_PRECISION) << m_number;
				return s.str().size();
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

	void value::cprint(MovingCharPointer& ptr, int depth, bool bPretty) const 
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
				std::ostringstream s;
				s << std::setprecision(JSON_NUMBER_PRECISION) << m_number;
				ptr.set(s.str().c_str(), s.str().size());
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

	void makeDepth(MovingCharPointer& ptr, int dep) {
		for (int i = 0; i < dep; i++) {
			ptr.set('\t');
		}
	}

	size_t array::psize(int depth, bool bPretty) const
	{
		int ret = 0;
		static int count = 0;
		count++;
		if (bPretty)
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
		if (bPretty) {
			ret++;
			ret += depth - 1;
		}
		ret++;
		return ret;
	}

	void array::cprint(MovingCharPointer& ptr, int depth, bool bPretty) const 
	{
		if (bPretty) {
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
		if (bPretty) {
			ptr.set('\n');
			makeDepth(ptr, depth - 1);
		}
		ptr.set(']');
	}

	size_t object::psize(int depth, bool bPretty) const 
	{
		int ret = 0;
		static int count = 0;
		count++;

		if (bPretty)
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
		if (bPretty) {
			ret += depth;
		}
		ret++;

		return ret;
	}

	void object::cprint(MovingCharPointer& ptr, int depth, bool bPretty) const 
	{
		if (bPretty) {
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
		if (bPretty) {
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
				std::string index = it->first;
				retVal[index] = retVal[index].merge(it->second);
			}
		} else if (isA(JSON_ARRAY) && V.isA(JSON_ARRAY)) {
			retVal = *this;
			arr->insert(arr->end(), V.arr->begin(), V.arr->end());
		} else if (isA(JSON_ARRAY) && V.isA(JSON_OBJECT)) {
			retVal = *this;
			retVal[(int)retVal.size()] = V;
		} else if (isA(JSON_OBJECT) && V.isA(JSON_ARRAY)) {
			retVal = V;
			retVal[(int)retVal.size()] = *this;
		} else {
			retVal = V;
		}

		return retVal;
	}

	void value::erase(size_t index) {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json erase: index %lu out of bounds", index);
			return;
		}
		if (arr) {
			if (index < arr->size()) {
				arr->erase(arr->begin() + index);
			}
		}
	}

	void value::erase(std::string index) {
		if (obj) {
			myMap::iterator it;
			it = obj->find(index);
			if (it != obj->end())
				obj->erase(it);
		}
	}
	
	void value::erase(iterator it) {
		if (it.IsArray() && arr) {
			arr->erase(it.arr());
		} else if (!it.None() && obj) {
			obj->erase(it.obj());
		}
	}

	void value::erase(iterator first, iterator last)
	{
		if (first.IsArray()  && last.IsArray() && arr) {
			arr->erase(first.arr(), last.arr());
		} else if (!first.None() && !last.None() && obj) {
			obj->erase(first.obj(), last.obj());
		}
	}

	bool value::exists(size_t index) {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json exists: index %lu out of bounds", index);
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
	
	bool value::exists(std::string index) {
		if (isA(JSON_OBJECT) && obj != NULL) {
			if (obj->empty()) {
				return false;
			}
			json::iterator it = obj->find(index);
			if (it != obj->end()) {
				switch((*it).isA()){
					default:
					case JSON_VOID:
						return false;
					case JSON_NULL:
					case JSON_BOOLEAN:
					case JSON_NUMBER:
					case JSON_STRING:
						return true;
					case JSON_ARRAY:
					case JSON_OBJECT:
						return !(*it).empty();
				}
			}
		}
		return false;
	}

	iterator value::insert(size_t index, value V)
	{
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json insert: index %lu out of bounds", index);
			return iterator();
		}
		if (myType != JSON_ARRAY) {
			m_number = 0;
			m_boolean = false;
			myType = JSON_ARRAY;
			if (!str.empty())
				str.clear();
			arr = new array();
			if (obj)
				delete obj;
			obj = NULL;
		}
        if (index <= arr->size()) {
			return iterator(arr->insert(arr->begin() + index, V));
		}
		return iterator();
	}
	
	iterator value::insert(std::string index, value V)
	{
		if (myType != JSON_OBJECT) {
			m_number = 0;
			m_boolean = false;
			myType = JSON_OBJECT;
			if (!str.empty())
				str.clear();
			obj = new object();
			if (arr)
				delete arr;
			arr = NULL;
		}
		return iterator(obj->insert(obj->end(), std::pair<std::string, value>(index, V)));
	}
	
	iterator value::insert(iterator position, value V) {
		if (!position.IsArray() || myType != JSON_ARRAY)
			return iterator();
		
		return iterator(arr->insert(position.arr(), V));
	}
	
	void value::insert(iterator position, iterator first, iterator last) {
		if (position.IsArray() && first.IsArray() && last.IsArray()) {
			if (myType != JSON_ARRAY || arr == NULL) {
				m_number = 0;
				m_boolean = false;
				myType = JSON_ARRAY;
				if (!str.empty())
					str.clear();
				arr = new array();
				if (obj)
					delete obj;
				obj = NULL;
			}
			arr->insert(position.arr(), first.arr(), last.arr());
		} else if (!position.IsArray() && !first.IsArray() && !first.None() && !last.IsArray() && !last.None()) {
			if (myType != JSON_OBJECT || obj == NULL) {
				m_number = 0;
				m_boolean = false;
				myType = JSON_OBJECT;
				if (!str.empty())
					str.clear();
				obj = new object();
				if (arr)
					delete arr;
				arr = NULL;
			}
			obj->insert(first.obj(), last.obj());
		}
	}
	
	void value::insert(iterator first, iterator last) {
		if (first.IsArray() && last.IsArray()) {
			if (myType != JSON_ARRAY) {
				m_number = 0;
				m_boolean = false;
				myType = JSON_ARRAY;
				if (!str.empty())
					str.clear();
				arr = new array();
				if (obj)
					delete obj;
				obj = NULL;
			}
			arr->insert(arr->end(), first.arr(), last.arr());
		} else if (!first.IsArray() && !first.None() && !last.IsArray() && !last.None()) {
			if (myType != JSON_OBJECT) {
				m_number = 0;
				m_boolean = false;
				myType = JSON_OBJECT;
				if (!str.empty())
					str.clear();
				obj = new object();
				if (arr)
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
				if (arr)
					delete arr;
				arr = NULL;
			}
			size_t i = 0;
			for (object::iterator it = obj->begin(); it != obj->end(); ++it) {
				if (i++ == index) {
					return it->first;
				}
			}
			return std::string();
		} else {
			return std::string();
		}
	}

	value::DEBUGPTR value::debug = NULL;

	value::value() {
		m_number = 0;
		m_boolean = false;

		myType = JSON_VOID;
		obj = NULL;
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}

	value::value(const value& V) {

		m_number = V.m_number;
		m_boolean = V.m_boolean;

		if (!V.str.empty()) {
			str.assign(V.str);
		}

		myType = V.myType;

		obj = NULL;
		if (V.obj) {
			obj = new object(V.obj);
		}

		arr = NULL;
		if (V.arr) {
			arr = new array(V.arr);
		}
		pParentObject = NULL;
		pParentArray = NULL;
	}
#ifdef __BORLANDC__
	value::value(document& V) {
#else
	value::value(const document& V) {
#endif
		m_number = V.m_number;
		m_boolean = V.m_boolean;

		if (!V.str.empty()) {
			str.assign(V.str);
		}

		myType = V.myType;

		obj = NULL;
		if (V.obj) {
			obj = new object(V.obj);
		}

		arr = NULL;
		if (V.arr) {
			arr = new array(V.arr);
		}
		pParentObject = NULL;
		pParentArray = NULL;
	}

	std::string value::typeName(JSONTypes type)
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
			if(myType != V.myType){
				switch(myType) {
					case JSON_NULL:
						debug("json operator= changed type from NULL to %s", typeName(V.myType).c_str());
						break;

					case JSON_BOOLEAN:
						debug("json operator= changed type from Boolean %i to %s", m_boolean, typeName(V.myType).c_str());
						break;

					case JSON_NUMBER:
						debug("json operator= changed type from Number %f to %s", m_number, typeName(V.myType).c_str());
						break;

					case JSON_STRING:
						debug("json operator= changed type from String %s to %s", str.c_str(), typeName(V.myType).c_str());
						break;

					case JSON_ARRAY:
						debug("json operator= changed type from Array to %s, orphanning:\n%s\n", typeName(V.myType).c_str(), this->print(0, true).c_str());
						break;

					case JSON_OBJECT:
						debug("json operator= changed type from Object to %s, orphanning:\n%s\n", typeName(V.myType).c_str(), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
		}

		m_number = V.m_number;
		m_boolean = V.m_boolean;

		str.assign(V.str);

		myType = V.myType;
		
		if (obj && V.obj == NULL){
			delete obj;
			obj = NULL;
		}
		if (V.obj && obj == NULL) {
			obj = new object(V.obj);
		} else if(V.obj) {
			*obj = *V.obj;
		}
		
		if (arr && V.arr == NULL){
			delete arr;
			arr = NULL;
		}
		if (V.arr && arr == NULL) {
			arr = new array(V.arr);
		} else if(V.arr){
			*arr = *V.arr;
		}
		if(myType != JSON_VOID){
			if(pParentObject){
				pParentObject->setNotEmpty();
				if(arr){
					arr->setParentObject(pParentObject);
				} else if (obj) {
					obj->setParentObject(pParentObject);
				}
			} else if(pParentArray){
				pParentArray->setNotEmpty();
				if(arr){
					arr->setParentArray(pParentArray);
				} else if (obj) {
					obj->setParentArray(pParentArray);
				}
			}
		}
		return *this;
	}
		
	value::value(bool V) {
		m_number = (double)V;
		m_boolean = !(V == 0);

		myType = JSON_BOOLEAN;
		obj = NULL;
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}

	value::value(const char* V) { 
		m_number = 0;
		m_boolean = false;

		if (V) {
			str.assign(V);
			myType = JSON_STRING;
		} else {
			myType = JSON_NULL;
		}

		obj = NULL;
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}
	value::value(char* V) { 
		m_number = 0;
		m_boolean = false;

		if (V) {
			str.assign(V);
			myType = JSON_STRING;
		} else {
			myType = JSON_NULL;
		}

		obj = NULL;
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}
	value::value(std::string V) {
		m_number = 0;
		m_boolean = false;
		str.assign(V);
		myType = JSON_STRING;
		obj = NULL;
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}
	value::value(object& V) { 
		m_number = 0;
		m_boolean = false;

		myType = JSON_OBJECT;
		obj = new object(V);
		arr = NULL;
		pParentObject = NULL;
		pParentArray = NULL;
	}
	value::value(array& V) {
		m_number = 0;
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
				if (arr->empty()) {
					return JSON_VOID;
				}
				break;
			}
			case JSON_OBJECT:
			{
				if (obj->empty()) {
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
		if (obj)
			delete obj;
		if (arr)
			delete arr;
	}

	value& value::at(size_t index)
	{
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json at: index %lu out of bounds", index);
			return *this;
		}
		if (myType == JSON_OBJECT) {
			if (index < obj->size()) {
				size_t iMyIndex = 0;
				for (object::iterator it = obj->begin(); it != obj->end(); ++it) {
					if (iMyIndex++ == index) {
						return it->second;
					}
				}
			}
		} else {
			return this[index];
		}
		return *this;
	}

	value& value::emptyArray()
	{
		if (myType != JSON_VOID) {
			if (debug) {
				switch(myType) {
					case JSON_NULL:
						debug("json emptyArray() changed type from NULL to Array.");
						break;

					case JSON_BOOLEAN:
						debug("json emptyArray() changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json emptyArray() changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json emptyArray() changed type from String %s to Array.", str.c_str());
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
			m_boolean = false;
			if (!str.empty())
				str.clear();
			if (obj)
				delete obj;
			obj = NULL;
			if (arr)
				delete arr;
			arr = NULL;
		}
		myType = JSON_ARRAY;
		arr = new array();
		if(pParentObject){
			arr->setParentObject(pParentObject);
		} else if(pParentArray){
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
					case JSON_NULL:
						debug("json emptyObject() changed type from NULL to Object.");
						break;

					case JSON_BOOLEAN:
						debug("json emptyObject() changed type from Boolean to Object.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json emptyObject() changed type from Number %f to Object.", m_number);
						break;

					case JSON_STRING:
						debug("json emptyObject() changed type from String %s to Object.", str.c_str());
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
			m_boolean = false;
			if (!str.empty())
				str.clear();
			if (obj)
				delete obj;
			obj = NULL;
			if (arr)
				delete arr;
			arr = NULL;
		}
		myType = JSON_OBJECT;
		obj = new object();
		if(pParentObject){
			obj->setParentObject(pParentObject);
		} else if(pParentArray){
			obj->setParentArray(pParentArray);
		}
		obj->setNotEmpty();
		return *this;
	}

	value & value::toArray()
	{
		if(myType == JSON_ARRAY){
			return *this;
		}
		value temp = *this;
		m_number = 0;
		m_boolean = false;
		str.clear();
		myType = JSON_ARRAY;
		if (obj)
			delete obj;
		obj = NULL;
		arr = new array();
		(*this)[0] = temp;
		return *this;
	}

	value& value::operator[](size_t index) {
		assert(i64(index) >= 0);
		if (index > size_t(-1) / size_t(2) - 1) {
			debug("json find: index %lu out of bounds", index);
			return *this;
		}
		if (arr) {
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
		}

		if (myType != JSON_VOID) {
			if (index == 0) {
				return *this;
			}
			if (debug) {
				switch(myType) {
					case JSON_NULL:
						debug("json operator[](size_t) changed type from NULL to Array.");
						break;

					case JSON_BOOLEAN:
						debug("json operator[](size_t) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json operator[](size_t) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json operator[](size_t) changed type from String %s to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json operator[](size_t) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_boolean = false;
			if (!str.empty())
				str.clear();
			if (obj)
				delete obj;
			obj = NULL;
		}

		myType = JSON_ARRAY;
		arr = new array();
        if(pParentObject){
            arr->setParentObject(pParentObject);
        } else if(pParentArray) {
            arr->setParentArray(pParentArray);
        }
		arr->resize(index + 1);
		value & ret = arr->at(index);
		ret.setParentArray(arr);
		return ret;
	}

	value& value::operator[](std::string index) {
		if (obj) {
			value& ret = obj->operator[](index);
			ret.setParentObject(obj);
			return ret;
		}
		if (myType != JSON_VOID) {
			if (debug) {
				switch(myType) {
					case JSON_NULL:
						debug("json operator[](std::string) changed type from NULL to Object.");
						break;

					case JSON_BOOLEAN:
						debug("json operator[](std::string) changed type from Boolean to Object.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json operator[](std::string) changed type from Number %f to Object.", m_number);
						break;

					case JSON_STRING:
						debug("json operator[](std::string) changed type from String %s to Object.", str.c_str());
						break;

					case JSON_ARRAY:
						debug("json operator[](std::string) changed type from Array to Object(%s), orphanning:\n%s\n", index.c_str(), this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_boolean = false;
			if (!str.empty())
				str.clear();
			if (arr)
				delete arr;
			arr = NULL;
		}
		myType = JSON_OBJECT;
		obj = new object();
        if(pParentObject){
            obj->setParentObject(pParentObject);
        } else if(pParentArray) {
            obj->setParentArray(pParentArray);
        }
		value & ret = obj->operator[](index);
		ret.setParentObject(obj);
		return ret;
	}

	void value::push_back(const value& val) {
		if (myType != JSON_ARRAY) {
			if (debug) {
				switch(myType) {
					case JSON_NULL:
						debug("json push_back(value val) changed type from NULL to Array.");
						break;

					case JSON_BOOLEAN:
						debug("json push_back(value val) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json push_back(value val) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json push_back(value val) changed type from String %s to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json push_back(value val) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_boolean = false;
			myType = JSON_ARRAY;
			if (!str.empty())
				str.clear();
			arr = new array();
			if (obj)
				delete obj;
			obj = NULL;
            if(pParentObject){
                arr->setParentObject(pParentObject);
            } else if(pParentArray) {
                arr->setParentArray(pParentArray);
            }
		}
		arr->emplace_back(val);
		arr->back().setParentArray(arr);

		if(val.myType != JSON_VOID){
			arr->setNotEmpty();
		}
	}
	
	void value::push_front(const value &val) {
		if (myType != JSON_ARRAY) {
			if (debug) {
				switch(myType) {
					case JSON_NULL:
						debug("json push_front(value val) changed type from NULL to Array.");
						break;

					case JSON_BOOLEAN:
						debug("json push_front(value val) changed type from Boolean to Array.", m_boolean);
						break;

					case JSON_NUMBER:
						debug("json push_front(value val) changed type from Number %f to Array.", m_number);
						break;

					case JSON_STRING:
						debug("json push_front(value val) changed type from String %s to Array.", str.c_str());
						break;

					case JSON_OBJECT:
						debug("json push_front(value val) changed type from Object to Array, orphanning:\n%s\n", this->print(0, true).c_str());
						break;

					default:
						break;
				}
			}
			m_number = 0;
			m_boolean = false;
			myType = JSON_ARRAY;
			if (!str.empty())
				str.clear();
			arr = new array();
			if (obj)
				delete obj;
			obj = NULL;
            if(pParentObject){
                arr->setParentObject(pParentObject);
            } else if(pParentArray) {
                arr->setParentArray(pParentArray);
            }
		}
		arr->emplace_front(val);
		arr->front().setParentArray(arr);
		if(val.myType != JSON_VOID){
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
	
	void value::resize(size_t iCount){
		if(myType == JSON_VOID){
			arr = new array();
		}
		if(arr){
			arr->resize(iCount);
		}
	}
	
	void value::resize(size_t iCount, value val){
		if(myType == JSON_VOID){
			arr = new array();
		}
		if(arr){
			arr->resize(iCount, val);
		}
	}

	bool value::pruneEmptyValues()
	{
		switch(myType){
			default:
			case JSON_VOID:
				return false;

			case JSON_NULL:
				m_number = 0;
				m_boolean = false;
				myType = JSON_VOID;
				str.clear();
				return false;

			case JSON_BOOLEAN:
				if(m_boolean == false){
					m_number = 0;
					myType = JSON_VOID;
					str.clear();
					return false;
				}
				return true;

			case JSON_NUMBER:
				if(m_number == 0.0){
					m_boolean = false;
					myType = JSON_VOID;
					str.clear();
					return false;
				}
				return true;

			case JSON_STRING:
				if(str.empty()){
					m_number = 0;
					m_boolean = false;
					myType = JSON_VOID;
					return false;
				}
				return true;

			case JSON_ARRAY:
			{
				bool bNotEmpty = false;
				for(reverse_iterator rit = (*this).rbegin(); rit != (*this).rend(); ++rit){
					if((*rit).isA(JSON_NULL) && bNotEmpty == true){
						continue;  // NULLs are placeholders in arrays and only ones after the last non null value are pruned.
					}
					if((*rit).pruneEmptyValues()){
						bNotEmpty = true;
					}
				}
				if(bNotEmpty == false){
					m_number = 0.0;
					m_boolean = false;
					str.clear();
					myType = JSON_VOID;
					if(obj){
						delete obj;
						obj = NULL;
					}
					if(arr){
						delete arr;
						arr = NULL;
					}
				}
				return bNotEmpty;
			}
			case JSON_OBJECT:
			{
				bool bNotEmpty = false;
				for(value &val : *this){
					if(val.pruneEmptyValues()){
						bNotEmpty = true;
					}
				}
				if(bNotEmpty == false){
					m_number = 0.0;
					m_boolean = false;
					str.clear();
					myType = JSON_VOID;
					if(obj){
						delete obj;
						obj = NULL;
					}
					if(arr){
						delete arr;
						arr = NULL;
					}
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
										
			case JSON_NULL:
			case JSON_VOID:
				return true;
				
			default:
				return false;
		}
	}
	
	bool array::empty() const
	{
		if(myVec::empty()){
			// bNotEmpty = false;
			return true;
		} else {
			return !bNotEmpty;
		}
	}

	void array::setNotEmpty() 
	{
		bNotEmpty = true;
		if(pParentArray){
			pParentArray->setNotEmpty();
		} else if(pParentObject){
			pParentObject->setNotEmpty();
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
        pParentObject = pSetTo;
        if (bNotEmpty && pParentObject) {
            pParentObject->setNotEmpty();
        }
    }
    
    void array::setParentArray(array * pSetTo)
    {
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
		// for (const std::pair<std::string, value> &pair: *this) {
		// 	if (!pair.second.empty()) {
		// 		return false;
		// 	}
		// }
		if(myMap::empty()){
			// bNotEmpty = false;
			return true;
		} else {
			return !bNotEmpty;
		}
        // return myMap::empty() && !bNotEmpty;
	}
	void object::setNotEmpty() 
	{
		bNotEmpty = true;
		if(pParentArray){
			pParentArray->setNotEmpty();
		} else if(pParentObject){
			pParentObject->setNotEmpty();
		}
	}

	size_t value::size() const
	{
		switch (myType) {
		case JSON_ARRAY:
		{
			size_t ret = arr->size();
			for (reverse_iterator it = rbegin(); it != rend(); ++it) {
				if ((*it).isA() == JSON_VOID) {
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
			for (const value & val : *this) {
				if (val.isA() != JSON_VOID) {
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

	size_t value::arraySize()
	{
		if(!isA(JSON_ARRAY))
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
			int iIndex = 0;
			for (it = arr->begin(); it != arr->end(); ++it) {
				if (searchFor.myType == JSON_OBJECT && it->myType == JSON_OBJECT) {
					if (bSubStr && ((*it)[getKey].myType == JSON_STRING && iGetType == JSON_STRING)) {
						if ((*it)[getKey].str.find(getVal.str) != std::string::npos) {
							retVal[iIndex++] = (*it);
						}
					} else {
						if ((*it)[getKey] == getVal) {
							retVal[iIndex++] = (*it);
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != std::string::npos) {
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
						if ((it->second)[getKey].str.find(getVal.str) != std::string::npos) {
							retVal[it->first] = it->second;
						}
					} else {
						if ((it->second)[getKey] == getVal) {
							retVal[it->first] = it->second;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->second.myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != std::string::npos) {
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
						if ((*it)[key].str.find(searchFor[key].str) != std::string::npos) {
							retVal++;
						}
					} else {
						if ((*it)[key] == searchFor[key]) {
							retVal++;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != std::string::npos) {
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
						if ((it->second)[key].str.find(searchFor[key].str) != std::string::npos) {
							retVal++;
						}
					} else {
						if ((it->second)[key] == searchFor[key]) {
							retVal++;
						}
					}
				} else if (searchFor.myType == JSON_STRING && it->second.myType == JSON_STRING) {
					if (bSubStr) {
						if (str.find(searchFor.str) != std::string::npos) {
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

	inline void value::threadDelete(object * obj)
	{
		// if(obj) {
			std::thread t(value::threadDeleteObjectWorker, obj);
			t.detach();
		// }
	}
	
	inline void value::threadDelete(array * arr)
	{
		// if(arr) {
			std::thread t(value::threadDeleteArrayWorker, arr);
			t.detach();
		// }
	}
	
	void value::threadDeleteObjectWorker(object* obj)
	{
//		if(obj){
			delete obj;
//		}
	}
	
	void value::threadDeleteArrayWorker(array* arr) {
//		if(arr){
			delete arr;
//		}
	}
		

	void value::clear() {
		if (arr){
			delete arr;
			arr = new array();
		}
		if (obj){
			delete obj;
			obj = new object();
		}
		if (!str.empty())
			str.clear();

		m_number = 0;
		m_boolean = false;
	}

	void value::threadedClear() {
		if (arr){
			threadDelete(arr);
			arr = new array();
		}
		if (obj){
			threadDelete(obj);
			obj = new object();
		}
		if (!str.empty())
			str.clear();

		m_number = 0;
		m_boolean = false;
	}

	void value::destroy() {
		m_number = 0;
		m_boolean = false;
		str.clear();
		myType = JSON_VOID;
		if (obj)
			delete obj;
		obj = NULL;
		if (arr)
			delete arr;
		arr = NULL;
	}

	void value::threadedDestroy() {
		m_number = 0;
		m_boolean = false;
		str.clear();
		myType = JSON_VOID;
		if (obj)
			threadDelete(obj);
		obj = NULL;
		if (arr)
			threadDelete(arr);
		arr = NULL;
	}

#ifdef __GNUC__
    void value::sort(bool (*compareFunc)(const value&, const value&)) {
        if (arr){
            DEBUGPTR oldDebug = debug;
            debug = NULL;
            std::sort(arr->begin(), arr->end(), compareFunc);
            debug = oldDebug;
        }
    }
#else
    void value::sort(bool (*compareFunc)(value&, value&)) {
        if (arr){
            DEBUGPTR oldDebug = debug;
            debug = NULL;
            std::sort(arr->begin(), arr->end(), compareFunc);
            debug = oldDebug;
        }
    }
#endif
    double value::number() {
		switch (myType) {
		case JSON_NUMBER:
			return m_number;

		case JSON_BOOLEAN:
			return m_boolean ? 1 : 0;

		case JSON_STRING: {
			std::istringstream convert(str);
			double d = 0;
			if (!(convert >> d))
				return 0;
			return d;
		}
		case JSON_OBJECT:
		{
			iterator it = (*this).find("#value");
			if (it != (*this).end()){
				return (*it).m_number;
			} else {
				str.erase();
			}
			return 0;
		}	
		default:
			return 0;
		}
	}

	i64 value::integer() {
		return (i64)number();
	}

	ui64 value::_uint64() {
		return (ui64)number();
	}

	float value::_float() {
		return (float)number();
	}

	long value::_long() {
		return (long)number();
	}

    int value::_int() {
        return (int)number();
    }

    size_t value::_size_t() {
        return (size_t)number();
    }

    short value::_short() {
		return (short)number();
	}

	char value::_char() {
		return (char)number();
	}

	unsigned long value::_ulong() {
		return (long)number();
	}

	unsigned int value::_uint() {
		return (int)number();
	}

	unsigned short value::_ushort() {
		return (short)number();
	}

	unsigned char value::_uchar() {
		return (char)number();
	}

	std::string& value::string() {
		switch (myType) {
		case JSON_STRING:
			break;

		case JSON_NUMBER: {
			if (str.empty()) {
				std::ostringstream s;
				s << std::setprecision(JSON_NUMBER_PRECISION) << m_number;
				str = s.str();
			}
			break;
		}

		case JSON_BOOLEAN:
			if (m_boolean)
				str = "true";
			else
				str = "false";
			break;

		case JSON_OBJECT:
		{
			iterator it = (*this).find("#value");
			if (it != (*this).end()){
				str.assign((*it).str);
			} else {
				str.erase();
			}
			break;
		}	

		default:
			str.erase();
			break;
		}
		return str;
	}

	const char* value::safeCString() {
		return string().c_str();
	}

	const char* value::c_str() {
		return string().c_str();
	}

	const char* value::cString() {
		switch (myType) {
		case JSON_VOID:
		case JSON_NULL:
			return NULL;
		default:
			return string().c_str();
		}
	}

	std::string value::print(int depth, bool bPretty) const
	{
		if (myType == JSON_ARRAY) {
			arr->resize(size());
		}
		size_t l = psize(depth, bPretty);
		MovingCharPointer ptr(l);
		cprint(ptr, depth, bPretty);
		return std::string(ptr.orig());
	}
	
	bool value::operator==(json::value V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return V.isA(JSON_VOID) || V.isA(JSON_NULL) || V.boolean() == false;

			case JSON_BOOLEAN:
				return m_boolean == V.boolean();
				
			case JSON_NUMBER:
				return m_number == V.number();
				
			case JSON_STRING:
				return str == V.string();
				
			case JSON_ARRAY:
			{
				if(V.myType != JSON_ARRAY)
					return false;
				return *arr == *(V.arr);
			}
			case JSON_OBJECT:
			{
				if(V.myType != JSON_OBJECT)
					return false;
				return *obj == *(V.obj);
			}
		}
	}
	
	bool value::operator!=(value V) const
	{
		return !(*this == V);
	}
	
	bool value::operator>(json::value V) const
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
				return str > V.string();
				
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
	
	bool value::operator<(json::value V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return false;
				
			case JSON_BOOLEAN:
				return m_boolean != V.boolean();
				
			case JSON_NUMBER:
				return m_number < V.number();
				
			case JSON_STRING:
				return str < V.string();
				
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
	
	bool value::operator<=(json::value V) const
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
				return str <= V.string();
				
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
	
	bool value::operator>=(json::value V) const
	{
		switch (myType) {
			case JSON_VOID:
			case JSON_NULL:
			default:
				return true;
				
			case JSON_BOOLEAN:
				return true;
				
			case JSON_NUMBER:
				return m_number >= V.number();
				
			case JSON_STRING:
				return str >= V.string();
				
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
	
	value value::operator+(json::value V) const
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
				return value(str + V.string());
				
			case JSON_ARRAY:
			case JSON_OBJECT:
			{
				value ret = *this;
				ret.insert(ret.end(), V.begin(), V.end());
				if(ret.obj){
					ret.obj->setNotEmpty();
				} else if(arr) {
					ret.arr->setNotEmpty();
				}
				return ret;
			}
		}
	}
	
	value value::operator-(json::value V) const
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
	
	value value::operator*(json::value V) const
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
	
	value value::operator/(json::value V) const
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
	
	value value::operator%(json::value V) const
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
	
	value& value::operator+=(json::value V)
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
				str += V.string();
				break;
				
			case JSON_ARRAY:
			case JSON_OBJECT:
				insert(end(), V.begin(), V.end());
				if(obj){
					obj->setNotEmpty();
				} else if(arr) {
					arr->setNotEmpty();
				}
				break;

			default:
				break;
		}
		return *this;
	}
	
	value& value::operator-=(json::value V)
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
	
	
	value &value::operator*=(json::value V)
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
	
	value &value::operator/=(json::value V)
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
	
	value &value::operator%=(json::value V)
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
	
	value value::operator-()
	{
		if (myType == JSON_NUMBER) {
			return value(-m_number);
		}
		return *this;
	}
	
	std::ostream& operator<<(std::ostream& S, document& doc) {
		S << doc.write(true);
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
			S << std::setprecision(JSON_NUMBER_PRECISION) << doc.number();
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
		strParseResult = V.strParseResult;
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
	bool document::parse(const std::string& inStr, PREPARSEPTR preParser, std::string preParseFileName) {
		return parse(inStr.c_str(), inStr.size(), preParser, preParseFileName);
	}

	bool document::parse(const char* inDat, size_t len, PREPARSEPTR preParser, std::string preParseFileName) {
		strParseResult = "Successful";
		bool bFailed = false;
		bParseSuccessful = true;
		if (arr)
			delete arr;
		arr = NULL;
		if (obj)
			delete obj;
		obj = NULL;
		myType = JSON_VOID;
		m_number = 0;
		m_boolean = false;
		str.clear();
		std::string sOut;
		const char * inStr = inDat;
		if (preParser != NULL) {
			std::string sDat(inDat, len);
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
		if(in.tell() >= in.size()){
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

	bool document::parseFile(std::string inStr, PREPARSEPTR preParser, bool bReWriteFile) {
		FILE* fd = fopen(inStr.c_str(), "rb");
		if (fd) {
			fseek(fd, 0, SEEK_END);
			size_t l = ftell(fd);
			fseek(fd, 0, SEEK_SET);
			char* buffer = new char[l + 1];

			buffer[l] = 0;
			fread(buffer, 1, l, fd);

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
			delete[] buffer;
			return bRetVal;
		}

		bParseSuccessful = false;
		return false;
	}

	std::string document::write(bool bPretty, PREWRITEPTR preWriter) const 
	{
		return write(1, bPretty, preWriter);
	}

	std::string document::write(int iDepth, bool bPretty, PREWRITEPTR preWriter) const
	{
		if(isA(JSON_VOID)){
			return "null";
		} else if (isA(JSON_OBJECT)) {
			size_t l = obj->psize(iDepth, bPretty);
			MovingCharPointer ptr(l);
			obj->cprint(ptr, iDepth, bPretty);
			if (preWriter == NULL) {
				return std::string(ptr.orig());
			} else {
				std::string sOut;
				return preWriter(std::string(ptr.orig()), sOut);
			}
		} else if (isA(JSON_ARRAY)) {
			arr->resize(size());
			size_t l = arr->psize(iDepth, bPretty);
			MovingCharPointer ptr(l);
			arr->cprint(ptr, iDepth, bPretty);

			if (preWriter == NULL) {
				return std::string(ptr.orig());
			} else {
				std::string sOut;
				return preWriter(std::string(ptr.orig()), sOut);
			}
		} else {
			size_t l = value::psize(iDepth, bPretty);
			MovingCharPointer ptr(l);
			value::cprint(ptr, iDepth, bPretty);
			std::string t(ptr.orig());
			if (preWriter == NULL) {
				return t;
			} else {
				std::string sDat(t);
				std::string sOut;
				return preWriter(sDat, sOut);
			}
		}
	}

	bool document::writeFile(std::string inStr, bool bPretty, PREWRITEPTR preWriter) const
	{
		FILE* fd = fopen(inStr.c_str(), "wb");
		if (fd) {
			std::string w = write(bPretty, preWriter);
			if(fwrite(w.data(), 1, w.size(), fd) != w.size()){
				debug("Failed Writing to %s.", inStr.c_str());
			}
			fclose(fd);
			return true;
		}
		return false;
	}

	int document::appendToArrayFile(std::string sFile, const document & atm, bool bPretty)
	{
		FILE* fd = fopen(sFile.c_str(), "r+b");
		// int iError = 0;
		if(!fd){
			fd = fopen(sFile.c_str(), "wb");
			if(fd){
				fputc('[', fd);
				if(bPretty){
					fputc('\n', fd);
					fputc('\t', fd);
				}
			}
		} else {
			int cFirst = fgetc(fd);
			if(cFirst == '['){
				fseek(fd, -1, SEEK_END);
				// iError = ferror(fd);
				while(fgetc(fd) != ']'){
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
				if(c != '['){
					fputc(',', fd);
					// iError = ferror(fd);
				}
				if(bPretty){
					fputc('\n', fd);
					// iError = ferror(fd);
					fputc('\t', fd);
					// iError = ferror(fd);
				}
			} else if(cFirst == EOF){
				fputc('[', fd);
				// iError = ferror(fd);
				if(bPretty){
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
		if(fd){
			std::string sNew = atm.write(2, bPretty);
			fwrite(sNew.data(), 1, sNew.size(), fd);
			// iError = ferror(fd);
			if(bPretty){
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
}

#if defined __BORLANDC__ && __BORLANDC__ < 0x0600
#pragma warn + 8026
#pragma warn + 8027
#endif
