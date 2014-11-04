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
  jDoc.writeFile("sample.json", true); // true is optioal and tells the writer to output "pretty" JSON.

