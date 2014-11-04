json::document
==============

The purpose of this library is to allow JSON to be used in C++ with a very simple interface similar to how you would use it in other languages that support it natively.

It is licensed under the MIT License.

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
- isa(json type) - returns true if the value's type is the same as that passed in the argument.  The types are the same as those returned from the isA() function above.
- empty() - returns false if an object or an array have items in them or if the value is a string, number or boolean.  Returns true if the value doesn't exist, if it's a NULL or if it is an empty object or array.

