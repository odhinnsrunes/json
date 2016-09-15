cmake_minimum_required(VERSION 2.8)

project(json_document)

option(ORDERED_JSON "Is the ordered json (ojson::document) class required" ON)
option(DATA_DOCUMENT "Is the xml functionality (data/odata::document) class(es) required" ON)


if(ORDERED_JSON)
add_library(ojson OBJECT json.cpp)
target_compile_definitions(ojson PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET ojson PROPERTY CXX_STANDARD 11)
SET(OJSON_OBJECTS $<TARGET_OBJECTS:ojson>)
endif(ORDERED_JSON)

add_library(ajson OBJECT json.cpp)
target_compile_definitions(ajson PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET ajson PROPERTY CXX_STANDARD 11)

if(DATA_DOCUMENT)
add_library(tinyxml OBJECT tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp)
if(ORDERED_JSON)
add_library(odata OBJECT data.cpp)
target_compile_definitions(odata PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET odata PROPERTY CXX_STANDARD 11)
SET(ODATA_OBJECTS $<TARGET_OBJECTS:odata>)
endif(ORDERED_JSON)
add_library(adata OBJECT data.cpp)
target_compile_definitions(adata PRIVATE SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET adata PROPERTY CXX_STANDARD 11)
SET(ADATA_OBJECTS $<TARGET_OBJECTS:adata> $<TARGET_OBJECTS:tinyxml>)
endif(DATA_DOCUMENT)


add_library(json STATIC $<TARGET_OBJECTS:ajson> ${OJSON_OBJECTS} ${ODATA_OBJECTS} ${ADATA_OBJECTS})
target_compile_definitions(json PUBLIC SUPPORT_ORDERED_JSON _CRT_SECURE_NO_WARNINGS)
set_property(TARGET json PROPERTY CXX_STANDARD 11)

if(DATA_DOCUMENT)
if(ORDERED_JSON)
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

install(TARGETS json2xml xml2json jsonpretty xmlpretty RUNTIME DESTINATION bin)
install(TARGETS json ARCHIVE DESTINATION lib)
install(FILES json.hpp data.hpp ojson.hpp odata.hpp DESTINATION include)

endif(SAMPLES)
endif(ORDERED_JSON)
endif(DATA_DOCUMENT)