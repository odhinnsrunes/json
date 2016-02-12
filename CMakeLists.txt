cmake_minimum_required(VERSION 3.4)

add_library(ojson OBJECT json.cpp)
target_compile_definitions(ojson PRIVATE _USE_ADDED_ORDER_ SUPPORT_ORDERED_JSON)
set_property(TARGET ojson PROPERTY CXX_STANDARD 11)

add_library(ajson OBJECT json.cpp)
target_compile_definitions(ajson PRIVATE SUPPORT_ORDERED_JSON)
set_property(TARGET ajson PROPERTY CXX_STANDARD 11)

add_library(json STATIC $<TARGET_OBJECTS:ojson> $<TARGET_OBJECTS:ajson>)
target_compile_definitions(json PUBLIC SUPPORT_ORDERED_JSON)
set_property(TARGET json PROPERTY CXX_STANDARD 11)

