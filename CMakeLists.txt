cmake_minimum_required(VERSION 3.17)
project(Parser)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_FILES /src/ts_parser.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(scr/header)

add_executable(Parser
        scr/header/tsCommon.h
        scr/header/tsTransportStream.h
        scr/TS_parser.cpp
        scr/tsTransportStream.cpp)
