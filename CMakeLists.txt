cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(DynamicFPS VERSION 1.0.0)

# Set up the mod binary
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    # Add any extra C++ source files here
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

if (APPLE)
    target_sources(${PROJECT_NAME} PRIVATE src/main.mm)
    set_source_files_properties(src/main.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    target_link_libraries(${PROJECT_NAME}
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

# Set up dependencies, resources, and link Geode.
setup_geode_mod(${PROJECT_NAME})
