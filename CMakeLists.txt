cmake_minimum_required(VERSION 3.10)
project(Alignment)

find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist Physics)
include(${ROOT_USE_FILE})

# Include headers
include_directories(include)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Automatically find .cc files in src/
file(GLOB SOURCES src/*.cc)

# Add executable
add_executable(time-alignment time-alignment.cpp ${SOURCES})
# Link against ROOT and MySQL libraries
target_link_libraries(time-alignment ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist ROOT::Physics SQLite::SQLite3)

# # Add executable
# add_executable(computeStripTimeDelayFromHistograms computeStripTimeDelayFromHistograms.cpp ${SOURCES})
# # Link against ROOT and MySQL libraries
# target_link_libraries(computeStripTimeDelayFromHistograms ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist SQLite::SQLite3)

# # Add executable
# add_executable(createTTreeForCorry createTTreeForCorry.cpp ${SOURCES})
# # Link against ROOT and MySQL libraries
# target_link_libraries(createTTreeForCorry ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist SQLite::SQLite3)
