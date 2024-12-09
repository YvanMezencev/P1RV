CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

project(P1RV_Project)

# Gather all source files
file(GLOB_RECURSE SRCS src/*)

# Specify the path to the GLUT include directory
set(GLUT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/GLUT/include")
set(GLUT_LIB_DIR "${CMAKE_SOURCE_DIR}/dependencies/GLUT/lib")
set(GLUT_BIN_DIR "${CMAKE_SOURCE_DIR}/dependencies/GLUT/bin")

# Specify the path to the stb_image include directory
set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/dependencies/stb_image")

# Specify the path to the the heightmaps directory
set(HEIGHTMAPS_DIR "${CMAKE_SOURCE_DIR}/heightmaps")
# Add executable
add_executable(P1RV_Project ${SRCS})

# Include directories
include_directories(${GLUT_INCLUDE_DIR} ${STB_INCLUDE_DIR} ${HEIGHTMAPS_DIR})

# Link GLUT library
target_link_options(P1RV_Project PRIVATE /SAFESEH:NO)

if(WIN32)
  target_link_libraries(P1RV_Project PRIVATE ${GLUT_LIB_DIR}/glut32.lib)
endif()

# Set C++ standard
if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET P1RV_Project PROPERTY CXX_STANDARD 20)
endif()