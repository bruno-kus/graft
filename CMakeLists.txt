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

file (GLOB_RECURSE DOMAIN CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/domain/*"
)

file (GLOB_RECURSE GRAFT-ASSOCIATOR CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/graft/associator/*"
)

file (GLOB_RECURSE UTIL CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*"
)
file (GLOB_RECURSE TESTS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*"
)
add_executable(graft-test
  ${DOMAIN}
  ${GRAFT-ASSOCIATOR}
  ${UTIL}
  ${TESTS}
  src/graft/graph.h
  src/graft/graft.h
  src/graft/member_buffer.h
  src/graft/object.h
  # tests/graft-test-main.cpp
)
