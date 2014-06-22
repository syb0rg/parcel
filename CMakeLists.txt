cmake_minimum_required(VERSION 2.8.7)
project(parcel)
set(CMAKE_C_FLAGS "-std=gnu11 -O3")

add_library(parcel parcel.c)
