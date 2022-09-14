cmake_minimum_required(VERSION 3.16.5)
project(NetworkMonitor)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} PRIVATE utils.hpp)

#set(SOURCE_FILES utils.hpp)

