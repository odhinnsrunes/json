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