/*
Copyright (c) 2012-2018 James Baker

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
#include <iomanip>
#include <fstream>

#define endStr "\n"

void write(const std::string &sPrefix, ojson::value & val, std::string & dest)
{
	switch (val.isA())
	{
		default:
		case json::JSON_VOID:
			break;

		case json::JSON_NULL:
		{
			dest.append(sPrefix);
			dest.append(" = (char*)NULL;");
			dest.append(endStr);
			break;
		}

		case json::JSON_BOOLEAN:
			if (val.boolean()) {
				dest.append(sPrefix);
				dest.append(" = true;");
				dest.append(endStr);
				break;
			} else {
				dest.append(sPrefix);
				dest.append(" = false;");
				dest.append(endStr);
				break;
			}

		case json::JSON_NUMBER:
			if (val.string().empty()) {
				std::ostringstream s;
				s << std::setprecision(JSON_NUMBER_PRECISION) << val.number();
				dest.append(sPrefix);
				dest.append(" = ");
				dest.append(s.str());
				dest.append(";");
				dest.append(endStr);
				break;
			} else {
				dest.append(sPrefix);
				dest.append(" = ");
				dest.append(val.string());
				dest.append(";");
				dest.append(endStr);
				break;
			}

		case json::JSON_STRING:
		{
			dest.append(sPrefix);
			dest.append(" = \"");
			dest.append(val.string());
			dest.append("\";");
			dest.append(endStr);
			break;
		}

		case json::JSON_ARRAY:
		{
			size_t iIndex = 0;
			for (ojson::value & subVal : val) {
				std::string sNewPrefix(sPrefix);
				sNewPrefix.reserve(sPrefix.size() + 24);
				sNewPrefix.append("[");
				sNewPrefix.append(std::to_string(iIndex++));
				sNewPrefix.append("]");
				write(sNewPrefix, subVal, dest);
			}

			break;
		}

		case json::JSON_OBJECT:
		{
			for (ojson::value & subVal : val) {
				std::string sNewPrefix(sPrefix);
				sNewPrefix.reserve(sPrefix.size() + subVal.key().size() + 2);
				sNewPrefix.append("[\"");
				sNewPrefix.append(subVal.key());
				sNewPrefix.append("\"]");
				write(sNewPrefix, subVal, dest);
			}
			break;
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

	odata::document jDoc;

	if (jDoc.parseFile(argv[1])) {
		std::string output;
		write(argv[2], jDoc, output);

		if (argc < 4) {
			std::cout << "json::document " << argv[2] << ";" << std::endl << std::endl;

			std::cout << output;
		} else {
			std::ofstream ofs (argv[3], std::ofstream::out);

			ofs << "json::document " << argv[2] << ";" << std::endl << std::endl;

  			ofs << output;

  			ofs.close();
		}
		return 0;		
	} else if (jDoc.parseXMLFile(argv[1])) {
		std::string output;
		write(argv[2], jDoc, output);

		if (argc < 4) {
			std::cout << "odata::document " << argv[2] << ";" << std::endl << std::endl;
			std::cout << argv[2] << ".rootTag(\"" << jDoc.rootTag() << "\");" << std::endl;
			std::cout << output;
		} else {
			std::ofstream ofs (argv[3], std::ofstream::out);

			ofs << "odata::document " << argv[2] << ";" << std::endl << std::endl;
			ofs << argv[2] << ".rootTag(\"" << jDoc.rootTag() << "\");" << std::endl;

  			ofs << output;

  			ofs.close();
		}
		return 0;		
	} else {
		std::cout << argv[0] << " could not open file: " << argv[1] << std::endl;
		return 2;
	}
}