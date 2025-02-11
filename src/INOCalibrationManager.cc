
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "INOCalibrationManager.h"

using namespace INO;

INOCalibrationManager& INOCalibrationManager::getInstance() {
  static INOCalibrationManager instance;
  return instance;
}

void INOCalibrationManager::writeTimeCalibration(const std::string& filename) {
  std::ofstream file(filename);
  if (!file) {
    std::cerr << "Error opening file for writing!" << std::endl;
    return;
  }
  // Write header
  file << "# Module Row Column Layer  Side  Strip  Offset\n";
  // Write data
  for (const auto &entry : timeCalibration) {
    auto stripId = entry.first;
    auto center = entry.second;
    file << std::setw(8) << stripId.module
         << std::setw(4) << stripId.row
         << std::setw(7) << stripId.column
         << std::setw(6) << stripId.layer
         << std::setw(6) << stripId.side
         << std::setw(7) << stripId.strip
         << std::setw(8) << center << '\n';
  }
  file.close();
}

void INOCalibrationManager::loadTimeCalibration(const std::string& filename) {
  std::ifstream file(filename);
  if (!file) {
    std::cerr << "Error opening file for reading!" << std::endl;
    return;
  }
  std::string line;
  // Skip header
  std::getline(file, line);
  // Read data
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    StripId stripId;
    double center;
    iss >> stripId.module >> stripId.row >> stripId.column >> stripId.layer
        >> stripId.side >> stripId.strip >> center;
    timeCalibration[stripId] = center;
  }
  file.close();

}
