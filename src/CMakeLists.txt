set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (PARCEL_SOURCE_FILES
    parcel.c
    )

add_library(source OBJECT ${PARCEL_SOURCE_FILES})
