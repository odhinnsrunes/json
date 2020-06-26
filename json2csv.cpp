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
#include "json2csv.hpp"

static std::string & csvEscape(std::string & in) 
{
	for (size_t i = 0; i < in.size(); i++) {
		if (in[i] == '"') {
			in.insert(i, 1, '"');
			i++;
		}
	}
	return in;
}

std::string & json2csv(std::string & sOut, ojson::value & data, bool bDoKeys)
{
	ojson::document arrangement;
	ojson::document object_keys;
	size_t iLine = 0;
	size_t len = 0;
	bool bIsObject = data.isA(ojson::JSON_OBJECT);
	if (bIsObject || data.isA(ojson::JSON_ARRAY)) {
		for (ojson::value & record : data) {
			if (record.isA(ojson::JSON_OBJECT)) {
				if (bIsObject) {
					object_keys[iLine] = record.key();
					len += record.key().size() + 3;
				}
				for (ojson::value & field : record) {
					if (!field.isA(ojson::JSON_OBJECT) && !field.isA(ojson::JSON_ARRAY) && !field.isA(ojson::JSON_VOID)) {
						arrangement[field.key()][iLine] = field;
						len += field.string().size() + 3;
					}
				}
				iLine++;
			} else {
				sOut = "";
				return sOut;
			}
		}
		sOut.reserve(len);
		if (bDoKeys) {
			bool bFirst = true;
			if (bIsObject) {
				sOut.append("\"object_key\"");
				bFirst = false;
			}
			for (ojson::value & val : arrangement) {
				if (!bFirst) {
					sOut.append(",");
				} else {
					bFirst = false;
				}
				sOut.append("\"");
				sOut.append(val.key());
				sOut.append("\"");
			}
			sOut.append("\n");
		}
		
		for (size_t i = 0; i < iLine; i++) {
			bool bFirst = true;
			if (bIsObject) {
				sOut.append("\"");
				std::string sEscaped(object_keys[i].string());
				sOut.append(csvEscape(sEscaped));
				sOut.append("\"");
				bFirst = false;
			}
			for (ojson::value fieldlist : arrangement) {
				if (!bFirst) {
					sOut.append(",");
				} else {
					bFirst = false;
				}
				ojson::value & field = fieldlist[i];
				switch (field.isA()) {
					case ojson::JSON_STRING:
					{
						sOut.append("\"");
						std::string sEscaped(field.string());
						sOut.append(csvEscape(sEscaped));
						sOut.append("\"");
						break;
					}
					case ojson::JSON_NUMBER:
					case ojson::JSON_BOOLEAN:
						sOut.append(field.string());
						break;

					default:
						break;
				}
			}
			sOut.append("\n");
		}
	} else {
		sOut = "";
		return sOut;
	}
	return sOut;
}

#if defined __STANDALONE_CSV__
#include <iostream>

int main(int argc, char** argv)
{
	if (argc < 3) {
		std::cerr << "Useage:" << std::endl;
		std::cerr << "\tjson2csv input_file.json output_file.csv [key1] [key2] ... [keyN]" << std::endl << std::endl;
		std::cerr << "NOTE: Optional keys allow you to convert a subset of a document." << std::endl;
		return 1;
	}
	std::string sOut;

	ojson::document jDoc;
	if (jDoc.parseFile(argv[1])) {

		ojson::value jWork = jDoc;
		for (int i = 3; i < argc; i++) {
			//std::cout << argv[i] << std::endl;
			ojson::value temp = jWork[argv[i]];
			jWork = temp;
		}
		json2csv(sOut, jWork, true);
		{
			FILE* fd = fopen(argv[2], "wb");
			if (fd) {
				if (fwrite(sOut.data(), 1, sOut.size(), fd) != sOut.size()) {
					std::cerr << "Failed Writing to " << argv[2] << "." << std::endl;
					fclose(fd);
					return 2;
				}
				fclose(fd);
			} else {
				std::cerr << "Failed to open " << argv[2] << "." << std::endl;
				return 3;
			}
		}
	} else {
		std::cerr << "Failed to parse " << argv[1] << "." << std::endl;
		return 4;
	}

	return 0;
}

#endif
