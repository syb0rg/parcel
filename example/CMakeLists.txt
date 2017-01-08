set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_SOURCE_DIR}/src)

set (EXAMPLE_SOURCE_FILES
    simple
)

add_executable(example ${EXAMPLE_SOURCE_FILES})
set_property(TARGET example
            PROPERTY POSITION_INDEPENDENT_CODE TRUE
            PROPERTY C_STANDARD 99
            PROPERTY C_STANDARD_REQUIRED TRUE
            )
target_link_libraries(example parcel)
