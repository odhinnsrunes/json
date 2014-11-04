json::document
==============

The purpose of this library is to allow JSON to be used in C++ with a very simple interface similar to how you would use it in other languages that support it natively.  It is also designed to be fast.  It can parse and write megabytes in milliseconds.  It's been tested in Linux, Windows and MacOS using g++, clang++, Visual C++ and even Borland C++.

The json::document library is licensed under the MIT License.

To use JSON in a c++ project, just include json.hpp and compile json.cpp with your own modules.

Included along side the json library are some extras that aren't required for normal use.  These are the data.?pp and jsonquery.?pp files.

data.?pp is an extension to json.?pp that allows a certain subset of XML to be parsed or written using the same syntax for accessing the data as the json library.

jsonquery.?pp is an experimental query language in JSON that is designed to get data from a JSON document much like an SQL statement would get it from a database.

Using json::document
--------------------

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
    
Although you can use size() and an index to ierate through a json::document array, it will not work for objects.  For both, it is better to use iterators:

    json::iterator itObject= jDoc["some_object"].find("some_sub_value");
    for(json::iterator it = (*itObject).begin(); it != (*itObject).end(); ++it){
        std::string sKey = it.key().string(); // this will work for arrays as well, but the key will always return 0.
                                              // key() returns a JSON atom (value).
        std::cout << "key = " << sKey << ", value = " << (*it).number() << "\n";
    }

or reverse_iterators:

    json::iterator itObject= jDoc["some_object"].find("some_sub_value");
    for(json::reverse_iterator rit = (*itObject).rbegin(); rit != (*itObject).rend(); ++rit){
        std::string sKey = rit.key().string(); // this will work for arrays as well, but the key will always return 0.
                                              // key() returns a JSON atom (value).
        std::cout << "key = " << sKey << ", value = " << (*rit).number() << "\n";
    }
    
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

You can get other information with teh following functions:

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
- pop\_back() - returns a json value (json::atom) that is the last item in an array and removes it from the array.  Returns an atom with an isA() type of json::JSON\_VOID if the array is empty.
- pop\_front() - returns a json value (json::atom) that is the first item in an array and removes it from the array.  Returns an atom with an isA() type of json::JSON\_VOID if the array is empty.
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

    if(jDoc["number"] == 6){} // works, however this is invalid: if(6 == jDoc["number"]) {} use instead: if(6 == jDoc["number"].number()) {}
    jDoc["string"] += ", world!";
    jDoc["number"]++;

