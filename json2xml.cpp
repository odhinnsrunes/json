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
#include "data.hpp"
#include <iostream>

bool ends_with(std::string value, std::string ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main (int argc, char ** argv)
{
	ojson::document args;
	ojson::document validArgs;

	if (argc != 3) {
		std::cout << "Useage:" << std::endl;
		std::cout << "\t" << argv[0] << " [input file name] [output file name]" << std::endl;
		return -1;

	}
	if (ends_with(argv[0], "jsonpretty")) {
		ojson::document in;
		if (in.parseFile(argv[1])) {
			if (!in.writeFile(argv[2], true)) {
				std::cout << "Couldn't open '" << argv[2] << "'' for writing." << std::endl;
				return 2;
			}
		} else {
			std::cout << "Couldn't open '" << argv[1] << "'' for parsing." << std::endl;
			std::cout << in.parseResult() << std::endl;
			return 1;
		}
	} else if (ends_with(argv[0], "xmlpretty")) {
		odata::document in;
		if (in.parseXMLFile(argv[1])) {
			if (!in.writeXMLFile(argv[2], true)) {
				std::cout << "Couldn't open '" << argv[2] << "'' for writing." << std::endl;
				return 2;
			}
		} else {
			std::cout << "Couldn't open '" << argv[1] << "'' for parsing." << std::endl;
			std::cout << in.parseResult() << std::endl;
			return 1;
		}
	} else if (ends_with(argv[0], "xml2json")) {
		odata::document in;
//		std::cout << "pass1" << std::endl;
//		in.parseXMLFile2(argv[1]);
//		std::cout << "pass2" << std::endl;
//		if (in.parseXMLFile2(argv[1])) {
//			if (!in.writeFile((std::string(argv[2]) + "new.json").c_str(), true)) {
//				std::cout << "Couldn't open '" << argv[2] << "'' for writing." << std::endl;
//				return 2;
//			}
//		}
//		std::cout << "passold" << std::endl;
		if (in.parseXMLFile(argv[1])) {
			if (!in.writeFile(argv[2], true)) {
				std::cout << "Couldn't open '" << argv[2] << "'' for writing." << std::endl;
				return 2;
			}
		} else {
			std::cout << "Couldn't open '" << argv[1] << "'' for parsing." << std::endl;
			std::cout << in.parseResult() << std::endl;
			return 1;
		}
	} else if (ends_with(argv[0], "json2xml")) {
		odata::document in;
		if (in.parseFile(argv[1])) {
			if (!in.writeXMLFile(argv[2], true)) {
				std::cout << "Couldn't open '" << argv[2] << "'' for writing." << std::endl;
				return 2;
			}
		} else {
			std::cout << "Couldn't open '" << argv[1] << "'' for parsing." << std::endl;
			std::cout << in.parseResult() << std::endl;
			return 1;
		}
	} else {
		std::cout << "Unkown command: " << argv[0] << std::endl;
		return -1;
	}
}
