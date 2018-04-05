# Copyright (c) 2012-2018 James Baker
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# The official repository for this library is at https://github.com/odhinnsrunes/json

cmake_minimum_required(VERSION 2.8)

project(json_document)

option(ORDERED_JSON "Is the ordered json (ojson::document) class required" ON)
option(DATA_DOCUMENT "Is the xml functionality (data/odata::document) class(es) required" ON)

if(STAND_ALONE)
set(OBJECT_TYPE OBJECT)
else(STAND_ALONE)
set(OBJECT_TYPE STATIC)
endif(STAND_ALONE)

if(ORDERED_JSON)
add_library(ojson ${OBJECT_TYPE} json.cpp)
target_compile_definitions(ojson PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET ojson PROPERTY CXX_STANDARD 11)
SET(OJSON_OBJECTS $<TARGET_OBJECTS:ojson>)
endif(ORDERED_JSON)

add_library(ajson ${OBJECT_TYPE} json.cpp)
target_compile_definitions(ajson PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET ajson PROPERTY CXX_STANDARD 11)

if(DATA_DOCUMENT)
add_library(tinyxml ${OBJECT_TYPE} tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp)
if(ORDERED_JSON)
add_library(odata ${OBJECT_TYPE} data.cpp)
target_compile_definitions(odata PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET odata PROPERTY CXX_STANDARD 11)
SET(ODATA_OBJECTS $<TARGET_OBJECTS:odata>)
endif(ORDERED_JSON)
add_library(adata ${OBJECT_TYPE} data.cpp)
target_compile_definitions(adata PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET adata PROPERTY CXX_STANDARD 11)
SET(ADATA_OBJECTS $<TARGET_OBJECTS:adata> $<TARGET_OBJECTS:tinyxml>)
endif(DATA_DOCUMENT)

if(STAND_ALONE)
add_library(json STATIC $<TARGET_OBJECTS:ajson> ${OJSON_OBJECTS} ${ODATA_OBJECTS} ${ADATA_OBJECTS})
target_compile_definitions(json PUBLIC SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET json PROPERTY CXX_STANDARD 11)

if(DATA_DOCUMENT)
if(ORDERED_JSON)

if(TEST_EXAMPLE)
add_executable(test test.cpp)
target_compile_definitions(test PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET test PROPERTY CXX_STANDARD 11)
target_link_libraries(test PRIVATE json)
endif(TEST_EXAMPLE)

if(SAMPLES)

add_executable(json2xml json2xml.cpp)
target_compile_definitions(json2xml PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET json2xml PROPERTY CXX_STANDARD 11)
target_link_libraries(json2xml PRIVATE json)

add_executable(xml2json json2xml.cpp)
target_compile_definitions(xml2json PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET xml2json PROPERTY CXX_STANDARD 11)
target_link_libraries(xml2json PRIVATE json)

add_executable(jsonpretty json2xml.cpp)
target_compile_definitions(jsonpretty PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET jsonpretty PROPERTY CXX_STANDARD 11)
target_link_libraries(jsonpretty PRIVATE json)

add_executable(xmlpretty json2xml.cpp)
target_compile_definitions(xmlpretty PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET xmlpretty PROPERTY CXX_STANDARD 11)
target_link_libraries(xmlpretty PRIVATE json)

add_executable(json2csv json2csv.cpp)
target_compile_definitions(json2csv PRIVATE __STANDALONE_CSV__ SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET json2csv PROPERTY CXX_STANDARD 11)
target_link_libraries(json2csv PRIVATE json)

install(TARGETS json2xml xml2json jsonpretty xmlpretty json2csv RUNTIME DESTINATION bin)

endif(SAMPLES)
install(FILES odata.hpp DESTINATION include)
endif(ORDERED_JSON)
install(FILES data.hpp DESTINATION include)
endif(DATA_DOCUMENT)

if(ORDERED_JSON)
install(FILES ojson.hpp arbitrary_order_map.hpp DESTINATION include)
endif(ORDERED_JSON)

install(FILES json.hpp DESTINATION include)

install(TARGETS json ARCHIVE DESTINATION lib)
endif(STAND_ALONE)
