cmake_minimum_required(VERSION 2.8)

option(ORDERED_JSON "Is the ordered json (ojson::document) class required" ON)
option(DATA_DOCUMENT "Is the xml functionality (data/odata::document) class(es) required" ON)


if(ORDERED_JSON)
add_library(ojson OBJECT json.cpp)
target_compile_definitions(ojson PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON)
set_property(TARGET ojson PROPERTY CXX_STANDARD 11)
SET(OJSON_OBJECTS $<TARGET_OBJECTS:ojson>)
endif(ORDERED_JSON)

add_library(ajson OBJECT json.cpp)
target_compile_definitions(ajson PRIVATE SUPPORT_ORDERED_JSON)
set_property(TARGET ajson PROPERTY CXX_STANDARD 11)

if(DATA_DOCUMENT)
add_library(tinyxml OBJECT tinyxml/tinystr.cpp tinyxml/tinyxml.cpp tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp)
if(ORDERED_JSON)
add_library(odata OBJECT data.cpp)
target_compile_definitions(odata PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON)
set_property(TARGET odata PROPERTY CXX_STANDARD 11)
SET(ODATA_OBJECTS $<TARGET_OBJECTS:odata>)
endif(ORDERED_JSON)
add_library(adata OBJECT data.cpp)
target_compile_definitions(adata PRIVATE SUPPORT_ORDERED_JSON)
set_property(TARGET adata PROPERTY CXX_STANDARD 11)
SET(ADATA_OBJECTS $<TARGET_OBJECTS:adata> $<TARGET_OBJECTS:tinyxml>)
endif(DATA_DOCUMENT)


add_library(json STATIC $<TARGET_OBJECTS:ajson> ${OJSON_OBJECTS} ${ODATA_OBJECTS} ${ADATA_OBJECTS})
target_compile_definitions(json PUBLIC SUPPORT_ORDERED_JSON)
set_property(TARGET json PROPERTY CXX_STANDARD 11)

