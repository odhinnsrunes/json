json::document
==============

The official repository for this library is at https://github.com/odhinnsrunes/json

The purpose of this library is to allow JSON to be used in C++ with a very simple interface similar to how you would use it in other languages that support it natively.  It is also designed to be fast.  It can parse and write megabytes in milliseconds.  It's been tested in Linux, Windows and MacOS using g++, clang++, Visual C++ and even Borland C++.

More information regarding the JSON standard can be found here: <http://json.org/>

The json::document library is licensed under the MIT License.  <http://opensource.org/licenses/MIT>

To use JSON in a c++ project, just include json.hpp and compile json.cpp with your own modules.

Included along side the json library are some extras that aren't required for normal use.  These are the data.?pp and jsonquery.?pp files.

data.?pp is an extension to json.?pp that allows a certain subset of XML to be parsed or written using the same syntax for accessing the data as the json library.

jsonquery.?pp is an experimental query language in JSON that is designed to get data from a JSON document much like an SQL statement would get it from a database.

This document is incomplete, but gets most of the information across.

Building json::document
-----------------------
The easiest way to build json::document is with CMake.

The following instructions should work on all systems, but you can tailor to taste:

1. Clone the repository:

    git clone https://github.com/odhinnsrunes/json.git json

2. Enter the local repository folder:

    cd json

3. Update Submodules:

    git submodule update --init --remote -- recursive

4. Create bukld folder:

    mkdir build

5. Enter build folder:

    cd build

6. Run CMake (the -D optins are not required, but turn on certain features described below.):

    cmake ../ -DORDERED_JSON -DDATA_DOCUMENT -DSAMPLES

7. Build:

    make
    
8. Install the library, header files and sample tools:

    make install

The build options are as follows:

- ORDERED_JSON - also create ojson::document, which instead of always outputting json with object keys in alphabetical order, it will output in the order the values were added.  This also includes the ability to insert key/value pairs anywhere in an object.
- DATA_DOCUMENT - also create data::document (and odata::document if ORDERED_JSON is selected).  This class can not only read and write json like json::document, but can also read and write a subset of XML (Where sub tags are not surrounded by multuple CDATA blocks) and can convert between JSON and XML.  Attributes will be stored in objects with '@' prepended to the attribute name and if there are attributes for a tag containing CDATA, the CDATA will be in a key value pair with the key "#value".
- SAMPLES - this will create some useful command line tools for conversion back and forth between JSON and XML and to make JSON and XML prettier by adding newlines and tabs to format the document.

Using json::document
--------------------

There are two ways to parse and two ways to write JSON documents:
- parse(string) - pass a JSON string and it will parse it.  This returns true if parsing was successful.  parseResult() will contain error information if parsing failed.
- parseFile(string) - will parse a file with the name given.  This returns true if parsing was successful.  parseResult() will contain error information if parsing failed.
- write(pretty) - will return a std::string containing the JSON document.  Pass true for pretty if you want the JSON formatted with indentation and line feeds or omit pretty or pass false to have the JSON without tabs or line feeds.
- writeFile(string, pretty) - will write the JSON document to the file passed as string.  Pretty works the same as above.  Will return true if the file could be opened and saved or false if the file could not be written.

Here is an example of reading and updating a simple JSON file with json::document:

    json::document jDoc;
    if(jDoc.parseFile("sample.json")){
        // read was successful
        printf("foo = %s\n", jDoc["bar"].c_str());
    } else {
        printf("Error reading JSON file: %s\n", jDoc.parseResult().c_str());
    }
    jDoc["bar"] = "baz";
    jDoc.writeFile("sample.json", true); // true is optional and tells the writer to output "pretty" JSON.

You can access arrays using a number as the index in the square brackets:

    for(size_t i = 0; i < jDoc.size(); i++){
        jDoc[i] = i * 2;
    }

You can access objects using a string or char array as the index in teh square brackets:

    jDoc["foo"] = 8;
    
Any numeric value, boolean, char array / string, object or array can be assigned to any JSON value:

    jDoc["null"] = (char*)NULL;
    jDoc["true"] = true;
    jDoc["false"] = false;
    jDoc["number"] = 1.234;
    jDoc["string"] = "Hello, world!";
    jDoc["object"] = jDoc["other_object"];
    jDoc["array"] = jDoc["oterh_array"];
    
You can retrive values from a json::document with several conversion functions:

- boolean() - returns a bool.
- number() - returns a double. (Converts from strings as appropriate.)
- integer() - returns a 64bit integer.  (Converts from strings as appropriate.)
- c_str() - returns a const char *.  Guaranteed to not be NULL.  (Converts from numbers as appropriate)
- string() - returns a std::string & (Converts from numbers as appropriate)
- cString() - returns a char *.  NULL if it doesn't exist or the JSON value is null. (Converts from numbers as appropriate)

Here are some examples:

    int i = (int)jDoc["integer"].integer();
    double d = jDoc["double"].number();
    std::string str = jDoc["string"].string();
    const char * szChar = jDoc["char"].c_str();

You can get other information with the following functions:

- size() - returns the number of elements in arrays or objects.  Numbers, strings and booleans return 1 and nulls and non-existant values return 0.
- exists(string or number) - returns true if the object or array contains the index value.  False otherwise.
- isA() - returns the type of value.  Can be:
    - json::JSON_VOID - value does not exist.
    - json::JSON_NULL - value is NULL.
    - json::JSON_BOOLEAN - value is a boolean.
    - json::JSON_NUMBER - value is a number.
    - json::JSON_STRING - value is a string.
    - json::JSON_ARRAY - value is an array.
    - json::JSON_OBJECT - value is an object.
- isA(json type) - returns true if the value's type is the same as that passed in the argument.  The types are the same as those returned from the isA() function above.
- empty() - returns false if an object or an array have items in them or if the value is a string, number or boolean.  Returns true if the value doesn't exist, if it's a NULL or if it is an empty object or array.

Some other functions for working with JSON are:
- emptyArray() - creates an empty array [] at the location specified.
- emptyObject() - creates an empty object {} at the location specified.
- push\_back(value) - pushes the value to the end of an array.  Creates the array if it doesn't exist.
- push\_front(value) - pushes the value to the start of an array.  Creates the array if it doesn't exist.
- pop\_back() - returns a json value (json::value) that is the last item in an array and removes it from the array.  Returns an value with an isA() type of json::JSON\_VOID if the array is empty.
- pop\_front() - returns a json value (json::value) that is the first item in an array and removes it from the array.  Returns an value with an isA() type of json::JSON\_VOID if the array is empty.
- insert(number, string or iterators and a value) - will insert a new value into an existing array or object at the point indicated.  This will invalidate any iterators referencing the array or object in question.  For an object, this is the same as just adding the value the normal way.

Some Examples of these are:

    jDoc["empty"].emptyArray();
    jDoc["empty"].emptyObject();
    jDoc["array"].push_back(6);
    jDoc["array"].push_front("Test");
    int i = (int)jDoc["array"].pop_back().integer();
    std::string s = jDoc["array"].pop_front().string();
    jDoc["array"].insert(0, "new value");

There are three functions for removing data from a json::document:
- clear() - this removes the contents of the value it is used on.
- erase(index, iterator or 2 iterators) - this finds the specified value or range of values and removes it.  The arguments can be either a number for removing an item from an array, a string for removing an item from an object, an iterator that will remove an item from an array or an object or tow iterators, one for the range start and one for the range end that will remove all items from an array or object that fall between the iterators.  Any iterator for the array or object will be invalidated by erase.
- destroy() - similar to erase, but removes the item it is called from.

For example:

    jDoc.clear(); // completely clears the json::document.
    jDoc["array"].clear(); // empties the array "array" but leaves it in the document as [].
    jDoc["array"].erase(6); // removes item 6 from "array".
    jDoc.erase("array"); //removes "array" completely from the document.
    jDoc["array"].destroy(); // the same as jDoc.erase("array");

json::document also has a full set of operators:
- +, - , *, /, %, +=, -=, *=, /=, %=  work on numbers, some work on booleans and + and += append strings like in std::string.
- ==, !=, <, >, <=, >= work on all value types.
- ++, -- and - work on numbers and booleans only.

For example:

    if(jDoc["number"] == 6){} // works
    if(6 == jDoc["number"]) {} // INVALID! Use this instead: 
    if(6 == jDoc["number"].number()) {}
    jDoc["string"] += ", world!";
    jDoc["number"]++;

Although you can use size() and an index to ierate through a json::document array, it will not work for objects.  For both, it is better to use iterators.  Iterators are extremely fast.  The functions for using them are the same as many of the standard container classes:
- begin() - returns an iterator pointing to the beginning od an array or object.  If a value is not an array or object it will return end().
- end() - returns an iterator pointint to the end of the array or object (technically it represents one past the end.)
- rbegin() - returns a reverse iterator pointing to the last item in an array or object.  If a value is not an array or object it will return rend().
- find(string or number index) - will return an iterator pointing to the given member of an array or object.  Will return end() if it isn't found or the value is not an array or object.
- rfind(string or number index) - will return a reverse_iterator pointing to the given member of an array or object.  Will return rend() if it isn't found or the value is not an array or object.

For normal iterators:

    json::iterator itObject= jDoc["some_object"].find("some_sub_value");
    for(json::iterator it = (*itObject).begin(); it != (*itObject).end(); ++it){
        std::string sKey = it.key().string(); // this will work for arrays as well, but the key will always return 0.
                                              // key() returns a JSON value (value).
        std::cout << "key = " << sKey << ", value = " << (*it).number() << "\n";
    }

For reverse_iterators:

    json::iterator itObject= jDoc["some_object"].find("some_sub_value");
    for(json::reverse_iterator rit = (*itObject).rbegin(); rit != (*itObject).rend(); ++rit){
        std::string sKey = rit.key().string(); // this will work for arrays as well, but the key will always return 0.
                                              // key() returns a JSON value (value).
        std::cout << "key = " << sKey << ", value = " << (*rit).number() << "\n";
    }

Real World Example
------------------

The following program will open a JSON file and resave it in a "pretty" format with indentation and line feeds.

    #include "json.hpp"
    #include <iostream>
    
    int main(int argc, char ** argv) {
        for(int i = 1; i < argc; i++){
            json::document jDoc;
            if(jDoc.parseFile(argv[i])){
                jDoc.writeFile(argv[i], true);
            } else {
                std::cout << "Failed to open file " << argv[i] << ": " < jDoc.parseResult() << "\n";
            }
        }
        return 0;
    }

LICENCE
-------
Copyright (c) 2012-2016 James Baker

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
