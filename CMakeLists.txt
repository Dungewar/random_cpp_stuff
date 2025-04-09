cmake_minimum_required(VERSION 3.14)
project(custom_testing)
set(CMAKE_CXX_STANDARD 23)

# Use SFML 3 static build from source
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Needed for static linking sometimes

# Point to custom-built SFML
set(SFML_DIR "$ENV{HOME}/sfml-install/lib/cmake/SFML")

# Add your executable
add_executable(custom_testing
        main.cpp
        SFMLMathThing.cpp
        SFMLMathThing.h
        font_data.cpp
        font_data.h
)

set(SFML_STATIC_LIBRARIES TRUE)
set(BUILD_SHARED_LIBS OFF)       # good to include
set(SFML_DIR "/usr/local/lib/cmake/SFML")  # or your custom-built path
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

target_link_libraries(custom_testing PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
)

# Optional: Put output in bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
