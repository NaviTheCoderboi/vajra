cmake_minimum_required(VERSION 4.0 )

set(PROJECT_NAME "vajra")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR src)
set(INCLUDE_DIR "include")

add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp)

file(GLOB_RECURSE SRC_FILES ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.h ${INCLUDE_DIR}/*.hpp)

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})