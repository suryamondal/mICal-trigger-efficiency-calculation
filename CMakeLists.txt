cmake_minimum_required(VERSION 3.10)
project(Alignment)

find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)
include(${ROOT_USE_FILE})

# Include the headers directory
include_directories(include)

# Automatically find all .cc files in the src directory
file(GLOB SOURCES src/*.cc)

# Add the executable with alignment.cpp outside
add_executable(alignment alignment.cpp ${SOURCES})
# Link against the necessary ROOT libraries
target_link_libraries(alignment ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist)
