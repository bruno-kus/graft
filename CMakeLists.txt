cmake_minimum_required(VERSION 3.28)
project(graft-mono)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

file (GLOB_RECURSE THIRD_PARTY CONFIGURE_DEPENDS
"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/*.h"
"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/*.hpp"
)
add_executable(graft-mono
    ${SOURCES}
    ${THIRD_PARTY}
    main.cpp
    # src/util/static-string.hpp
)
