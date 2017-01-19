#include "json.hpp"
#include <iostream>

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
#define MYOTHERKEY(z) JSONENUMKEY(myOtherKeys, z)

template <class T>
void test(const char * type)
{
	// bStarted = true;
	cout << "Testing: " << type << endl;
	T jEnum;

	jEnum[MYKEY(one)] = one;
	jEnum[MYKEY(two)] = two;
	jEnum[MYKEY(three)] = three;
	jEnum[MYOTHERKEY(four)] = four; // works


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
		lpad(k1, '0', 3);
		for (size_t j = 0; j < 10; j++) {
			string k2 = to_string(j);
			lpad(k2, '0', 3);
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
	cout << "Write File 3:               ";
	if (j3.writeFile("test3.json", true)) {
		cout << "Success!" << endl;
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
	TEST(json::document);
	TEST(ojson::document);
	return 0;
}
