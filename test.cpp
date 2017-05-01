#include "data.hpp"
#include <iostream>
#include <stdarg.h>

using namespace std;

enum myKeys{
	one = 1,
	two,
	three
};

enum myOtherKeys{
	four = 4,
	five,
	six,
	seven
};

#define MYKEY(z) JSONENUMKEY(myKeys, z)
#define MYOTHERKEY(z) DATAATTENUMKEY(myOtherKeys, z)

void Debug(const char * format, ...)
{
	std::string s;

	size_t 	n,
			size = 100;

	bool b = false;

	va_list marker;

	while (!b) {
		s.resize(size);
		va_start(marker, format);
		n = vsnprintf((char*)s.c_str(), size, format, marker);
		va_end(marker);
		b = (n < size);
		if (n > 0 && n != (size_t)-1 && b) {
			size = n;
		} else if (n == (size_t)-1) {
			size = size * 2; // stupid nonconformant microsoft
		} else {
			size = n * 2;
		}
	}
	printf("%s", s.c_str());	
}

template <class T>
void test(const char * type)
{
	// bStarted = true;
	cout << "Testing: " << type << endl;
	data::document dData;
	cout << "Parse XML File 1:           ";
	if (dData.parseXMLFile("PushKeyData.SETX")) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}
	cout << "Parse XML File 2:           ";
	if (dData.parseXMLFile("PushKeyData.SETX")) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}
	T jEnum;

	jEnum[MYKEY(one)] = one;
	jEnum[MYKEY(two)] = two;
	jEnum[DATA_ATT(MYKEY(three))] = three;
	jEnum[MYOTHERKEY(four)] = four; // works

	jEnum.writeFile("testEnum.json", true);
	T j1;
	cout << "Parse File 1:               ";
	if (j1.parseFile("test1.json")) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}

	cout << "Build                       ";
	T j2;
	for (size_t i = 0; i < 10; i++) {
		string k1 = to_string(i);
		// lpad(k1, '0', 3);
		for (size_t j = 0; j < 10; j++) {
			string k2 = to_string(j);
			// lpad(k2, '0', 3);
			for (size_t k = 0; k < 10; k++) {
				j2[k1][k2]["a"].push_back(k);
				j2[k1][k2]["b"][k] = k;
			}
		}
	}
	cout << "Done!" << endl;;


	cout << "Compare ==:                 ";
	if (j1 == j2) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}

	cout << "Write File 2:               ";
	if (j2.writeFile("test2.json", true)) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}

	T j3;
	j3 = j2;
	for (auto it = j3.begin(); it != j3.end(); it++) {
		cout << " - " << (*it)["001"]["a"][2]._int() << " - " << endl;
	}
	cout << "Write File 3:               ";
	if (j3.writeFile("test3.json", true)) {
		cout << "Success!" << endl;
	} else {
		cout << "Failed!" << endl;
	}

	json::document j4;
	j4.push_back("test");
	cout << "push_back                   ";
	if (j4[0] == "test") {
		cout << "Success!" << endl;
		cout << j4[0].string() << endl;
	} else {
		cout << "Failed!" << endl;
	}

	cout << endl;
}	


std::string & lpad(std::string & in, char with, size_t length)
{
	if (length > in.size()) {
		return in.insert(0, length - in.size(), with);
	} else {
		return in;
	}
}

#define TEST(x) test<x>(#x)

int main(int, char**)
{
	json::value::setDebug(Debug);
	TEST(json::document);
	TEST(ojson::document);
	return 0;
}