cmake_minimum_required(VERSION 3.14)
project(custom_testing)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# NOTE: Do NOT hardcode SFML_DIR here.
# Instead, pass it in from the command line via -DSFML_DIR=...
# That way it works for GitHub Actions, macOS, Linux, etc.

# Optional: Help CMake find SFML
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${SFML_DIR}")

# Use static libraries
set(BUILD_SHARED_LIBS OFF)
set(SFML_STATIC_LIBRARIES TRUE)

# Find SFML
find_package(SFML REQUIRED COMPONENTS Graphics Window System)

# Build executable
add_executable(custom_testing
        main.cpp
        SFMLMathThing.cpp
        SFMLMathThing.h
        font_data.cpp
        font_data.h
)

target_link_libraries(custom_testing PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)

# Output binary in bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
