cmake_minimum_required(VERSION 3.1)
project(tiger)

set(CMAKE_CXX_STANDARD 20)

# Bison/Flex stuff documented here: https://cmake.org/cmake/help/v2.8.8/cmake.html#module:FindFLEX
find_package(BISON)
find_package(FLEX)

BISON_TARGET(TigerParser tiger-grammar.yy ${CMAKE_CURRENT_BINARY_DIR}/tiger-grammar.tab.cpp)
FLEX_TARGET(TigerScanner tiger-lex.ll  ${CMAKE_CURRENT_BINARY_DIR}/tiger-lex.yy.cpp)
ADD_FLEX_BISON_DEPENDENCY(TigerScanner TigerParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
# Thanks, cmake.org., for the above and ...OUTPUTS dependencies on the list below
   
add_executable(tiger
  ${BISON_TigerParser_OUTPUTS}
  ${FLEX_TigerScanner_OUTPUTS}
  AST
  AST-print-attributes
  AST-print
  depth
  errormsg
  height
  HERA_code
  layout_frames
  parent
  result_reg
  static_checks
  ST
  tiger
  tigerParseDriver
  typecheck
  types
  util
)

include_directories(/home/courses/include ../HaverfordCS/include)
