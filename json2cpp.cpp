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
#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef _WIN32
#define endStr "\r\n"
#else
#define endStr "\n"
#endif

std::string write(const std::string &sPrefix, json::value & val)
{
	switch (val.isA())
	{
		default:
		case json::JSON_VOID:
			return std::string();

		case json::JSON_NULL:
			return sPrefix + " = (char*)NULL;" + endStr;

		case json::JSON_BOOLEAN:
			if (val.boolean()) {
				return sPrefix + " = true;" + endStr;
			} else {
				return sPrefix + " = false;" + endStr;
			}

		case json::JSON_NUMBER:
			if (val.string().empty()) {
				std::ostringstream s;
				s << std::setprecision(JSON_NUMBER_PRECISION) << val.number();
				return sPrefix + " = " + s.str() + ";" + endStr;
			} else {
				return sPrefix + " = " + val.string() + ";" + endStr;
			}

		case json::JSON_STRING:
			return sPrefix + " = \"" + val.string() + "\";" + endStr;

		case json::JSON_ARRAY:
		{
			std::string ret;
			size_t iIndex = 0;
			for (json::value & subVal : val) {
				std::string sNewPrefix = sPrefix + "[" + std::to_string(iIndex++) + "]";
				ret.append(write(sNewPrefix, subVal));
			}
			return ret;
		}

		case json::JSON_OBJECT:
		{
			std::string ret;
			for (json::value & subVal : val) {
				std::string sNewPrefix = sPrefix + "[\"" + subVal.key() + "\"]";
				ret.append(write(sNewPrefix, subVal));
			}
			return ret;
		}

	}
}

int main(int argc, char const *argv[])
{
	if (argc != 3 && argc != 4) {
		std::cout << argv[0] << " must take 2 or 3 arguments:" << std::endl;
		std::cout << "\t" << argv[0] << " [json file name] [json::document variable name] {optional output file name}" << std::endl;
		std::cout << "\t" << "If no output file name is given, output will be sent to stdout." << std::endl;
		return 1;
	}

	json::document jDoc;

	if (jDoc.parseFile(argv[1])) {
		if (argc < 4) {
			std::cout << write(argv[2], jDoc);
		} else {
			std::ofstream ofs (argv[3], std::ofstream::out);

  			ofs << write(argv[2], jDoc);

  			ofs.close();
		}
		return 0;		
	} else {
		std::cout << argv[0] << " could not open file: " << argv[1] << std::endl;
		return 2;
	}
}