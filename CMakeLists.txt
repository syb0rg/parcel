# minimum required version for C_STANDARD
cmake_minimum_required(VERSION 3.1.3)
set(PROJECT_NAME parcel)
project(${PROJECT_NAME})

# set properties for all targets
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE CACHE Release FORCE)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 --coverage")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

add_subdirectory(src)
#add_subdirectory(example)

add_library(${PROJECT_NAME}_static STATIC
    $<TARGET_OBJECTS:source>
)
add_library(${PROJECT_NAME} SHARED
    $<TARGET_OBJECTS:source>
)
