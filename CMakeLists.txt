cmake_minimum_required(VERSION 3.1)
project(tiger)

set(CMAKE_CXX_STANDARD 20)

# Bison/Flex stuff documented here: https://cmake.org/cmake/help/v2.8.8/cmake.html#module:FindFLEX
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix bison
            RESULT_VARIABLE BREW_BISON
            OUTPUT_VARIABLE BREW_BISON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    endif()

    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
    endif()
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(TigerParser tiger-grammar.yy ${CMAKE_CURRENT_BINARY_DIR}/tiger-grammar.tab.cpp)
FLEX_TARGET(TigerScanner tiger-lex.ll  ${CMAKE_CURRENT_BINARY_DIR}/tiger-lex.yy.cpp)
ADD_FLEX_BISON_DEPENDENCY(TigerScanner TigerParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
# Thanks, cmake.org., for the above and ...OUTPUTS dependencies on the list below
#   but, note I added CMAKE_CURRENT_SOURCE_DIR to the above
   
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
        HERA_data.cpp)

include_directories(/home/courses/include ../HaverfordCS/include)
