cmake_minimum_required(VERSION 3.0.0)
project(06_lua VERSION 1.68.0 DESCRIPTION "06_lua sample for Wolf")

if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# set the default path lib
if(UNIX)
    if(APPLE)
        # APPLE OSX
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bin/osx/)
    else()
        # LINUX
        if (CMAKE_BUILD_TYPE MATCHES Debug)
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bin/linux/x64/debug/)
        else()
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bin/linux/x64/release/)
        endif()
    endif()
endif()

set(CMAKE_C_COMPILER "clang")#gcc
set(CMAKE_CXX_COMPILER "clang++")#g++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXE_LINKER_FLAGS    "-Wl,--as-needed ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_SHARED_LINKER_FLAGS}")

add_executable(06_lua 
main.cpp
pch.cpp)

# includes
include(CPack)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../../../../engine/src/wolf.system/
${CMAKE_CURRENT_SOURCE_DIR}/../../../../engine/deps/luaJIT/include/)

# pre processors
target_compile_definitions(06_lua PUBLIC 
_GNU_SOURCE 
_POSIX_PTHREAD_SEMANTICS 
_REENTRANT 
_THREAD_SAFE
__LUA__ 
__linux
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(06_lua PUBLIC _DEBUG DEBUG) 
endif()

# compiler options
target_compile_options(06_lua PRIVATE -fPIC -m64)

# libs
link_directories(/usr/local/lib)
if (CMAKE_BUILD_TYPE MATCHES Debug)
target_link_libraries(06_lua ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bin/linux/x64/debug/libwolf.system.linux.so)
else()
target_link_libraries(06_lua ${CMAKE_CURRENT_SOURCE_DIR}/../../../../bin/linux/x64/release/libwolf.system.linux.so)
endif()

target_link_libraries(06_lua anl rt nsl pthread dl)
