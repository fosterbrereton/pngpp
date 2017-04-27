project (pngpp)
cmake_minimum_required (VERSION 2.6)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++14")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
set(CMAKE_SKIP_RPATH OFF)

file(GLOB APP_SRC ./src/*.cpp)

get_filename_component(APP_HEADERS_PATH ./include ABSOLUTE)

include_directories(pngpp ${APP_HEADERS_PATH} ${CONAN_INCLUDE_DIRS})
link_directories(${CONAN_LIB_DIRS})

add_executable(pngpp ${APP_SRC})

target_link_libraries(pngpp ${CONAN_LIBS})

set_target_properties(pngpp PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++14)
set_target_properties(pngpp PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)
