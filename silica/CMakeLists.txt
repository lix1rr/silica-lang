cmake_minimum_required(VERSION 3.4.3)
project(Silica)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)



set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(SilicaJIT  "ast/ast.cpp"  "prism library/prism-lib.cpp" "parsing/Parser.cpp" "parsing/tokens.cpp"  "main.cpp"  "ast/types.h" )

target_compile_definitions(SilicaJIT PRIVATE PROJECT_DIR="${PROJECT_SOURCE_DIR}" TARGET_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}" TARGET_OS="${CMAKE_SYSTEM_NAME}" )
target_include_directories(SilicaJIT PRIVATE "include")

if(MSVC)
  target_compile_options(SilicaJIT PRIVATE /W4)
else()
  target_compile_options(SilicaJIT PRIVATE -Wall -Wextra -pedantic -Wno-reorder-ctor)
endif()