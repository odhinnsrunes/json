json::document
==============

The purpose of this library is to allow JSON to be used in C++ with a very simple interface similar to how you would use it in other languages that support it natively.

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
        std::string sKey = it.key().string();
        std::cout << "key = " << sKey << ", value = " << (*it).number() << "\n";
    }
    
You can retrive values from a json::document with several conversion functions:

- boolean() - returns a bool.
- number() - returns a double. (Converts from strings as appropriate.)
- integer() - returns a 64bit integer.
- c_str() - returns a const char *.  Guaranteed to not be NULL.
- string() - returns a std::string &
- cString() - returns a char *.  NULL if it doesn't exist or the JSON value is null.

    
