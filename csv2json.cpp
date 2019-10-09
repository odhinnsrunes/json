#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#define _USE_ADDED_ORDER_
#include "data.hpp"

std::vector< std::vector<std::string> > CSVData;

std::string RTrim(const std::string &in) 
{
	std::string ret(in);
	size_t l = ret.find_last_not_of(" \t\r\n");
	if (l != std::string::npos && l < ret.size()) {
		ret.resize(l + 1);
	}
	return ret;
}

std::string deQuote(std::string str)
{
	if (str[0] == '\"') {
		str = str.substr(1);
	}
	if (str[str.length() - 1] == '\"') {
		str = str.substr(0, str.length() - 1);
	}
	return str;
}

size_t ParseCSV(const char * szFileName)
{
	FILE * fp;

	size_t iLines = 0;

	fp = fopen(szFileName, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t l = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		char * szData = new char[l + 1];

		char * szField = szData;

		memset(szData, 0, l + 1);
		if (fread(szData, 1, l, fp) != l) {
			printf("Error reading file.");
		}
		fclose(fp);

		bool bQuote = false;
		
		size_t iFields = 0;

		for (size_t i = 0; i < l; i++) {
			if (bQuote) {
				if (szData[i] == '\"') {
					bQuote = false;
				}
			} else {
				if (szData[i] == '\"') {
					bQuote = true;
					continue;
				} else if (szData[i] == ',') {
					printf("Got Comma %i characters %i lines %i fields\n", (int)i, (int)iLines, (int)iFields);
					if (iLines >= CSVData.size()) {
						CSVData.resize(iLines + 1);
					}
					CSVData[iLines].push_back(deQuote(std::string(szField, &szData[i] - szField)));
					szField = &szData[i] + 1;
					iFields++;
				} else if (szData[i] == '\n') {
					printf("Got newline %i characters %i lines %i fields\n", (int)i, (int)iLines, (int)iFields);
					if (iLines >= CSVData.size()) {
						CSVData.resize(iLines + 1);
					}
					CSVData[iLines].push_back(deQuote(std::string(szField, &szData[i] - szField)));
					iFields = 0;
					szField = &szData[i] + 1;
					iLines++;
				} else if (i == l - 1) {
					printf("Got end %i characters %i lines %i fields\n", (int)i, (int)iLines, (int)iFields);
					if (iLines >= CSVData.size()) {
						CSVData.resize(iLines + 1);
					}
					CSVData[iLines].push_back(deQuote(std::string(szField)));
					iFields = 0;
					// szField = &szData[i] + 1;
					iLines++;
				}
			}
		}
		delete [] szData;
	}
	return iLines;
}

int main(int argc, char ** argv) 
{
	if (std::string(argv[0]) == "csv2xml") {
		if (argc != 4) {
			printf("Useage: %s input.csv output.json root_tag\n", argv[0]);
			return 1;
		}
	} else {
		if (argc != 3) {
			printf("Useage: %s input.csv output.json \n", argv[0]);
			return 1;
		}
	}
	size_t iLines = ParseCSV(argv[1]);

	for (size_t i = 1; i < iLines; i++) {
		odata::document outDoc;

		for (size_t j = 1; j < CSVData[i].size(); j++) {
			outDoc[CSVData[0][j]] = RTrim(CSVData[i][j]);
		}
		if (std::string(argv[0]) == "csv2xml") {
			outDoc.writeXMLFile(argv[2], argv[3], true);
		} else {
			outDoc.writeFile(argv[2], true);
		}
	}
	return 0;

}
