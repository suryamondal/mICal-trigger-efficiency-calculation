cmake_minimum_required(VERSION 3.10)
project(Alignment)

find_package(ROOT REQUIRED COMPONENTS Core RIO Tree Hist)  # Add Hist component
include(${ROOT_USE_FILE})

set(SOURCES
  SNM.C
  INOEvent.cc
  INOTimeGroupingModule.cc
  INOCalibrationManager.cc
  alignment.cpp
  )

add_executable(alignment ${SOURCES})

# Link against the necessary ROOT libraries
target_link_libraries(alignment ROOT::Core ROOT::RIO ROOT::Tree ROOT::Hist)
