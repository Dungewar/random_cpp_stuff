cmake_minimum_required(VERSION 3.14)
project(custom_testing)
set(CMAKE_CXX_STANDARD 23)

# WINDOWS
if(WIN32)
    # Manually set SFML_DIR to where you extracted SFML 3.x
    set(SFML_DIR "C:/SFML/lib/cmake/SFML")  # UPDATE THIS IF NEEDED
endif()


set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

# FIND SFML 3.x
find_package(SFML 3.0.0 REQUIRED COMPONENTS Graphics Window System Audio)

# Add the executable
add_executable(
    custom_testing    main.cpp
        SFMLMathThing.cpp
        SFMLMathThing.h
)

# Copy the 'resources' folder into the build directory
#file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})

# Link to SFML 3.x modern targets
target_link_libraries(custom_testing PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
)