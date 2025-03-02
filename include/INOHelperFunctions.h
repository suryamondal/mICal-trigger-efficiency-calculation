#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <INOStructs.h>

namespace INO {

  std::string getStripName(const StripId& stripId) {
    return "m" + std::to_string(stripId.module) +
      "_r" + std::to_string(stripId.row) +
      "_c" + std::to_string(stripId.column) +
      "_l" + std::to_string(stripId.layer) +
      "_" + (stripId.side ? "x" : "y") +
      "_s" + std::to_string(stripId.strip);
  };

  std::string getSideName(const SideId& sideId) {
    return "m" + std::to_string(sideId.module) +
      "_r" + std::to_string(sideId.row) +
      "_c" + std::to_string(sideId.column) +
      "_l" + std::to_string(sideId.layer) +
      "_" + (sideId.side ? "x" : "y");
  };

  void writePixelsToFile(const std::string& filename, const std::vector<std::vector<INO::PixelId>>& allPixels) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
      std::cerr << "Error opening file for writing: " << filename << std::endl;
      return;
    }
    // Write the outer vector size
    size_t outerSize = allPixels.size();
    file.write(reinterpret_cast<const char*>(&outerSize), sizeof(outerSize));
    for (const auto& innerVec : allPixels) {
      // Write the inner vector size
      size_t innerSize = innerVec.size();
      file.write(reinterpret_cast<const char*>(&innerSize), sizeof(innerSize));
      // Write the actual pixel data
      file.write(reinterpret_cast<const char*>(innerVec.data()), innerSize * sizeof(INO::PixelId));
    }
    file.close();
  }

  void readPixelsFromFile(const std::string& filename, std::vector<std::vector<INO::PixelId>>& allPixels) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      std::cerr << "Error opening file for reading: " << filename << std::endl;
      return;
    }
    size_t outerSize;
    file.read(reinterpret_cast<char*>(&outerSize), sizeof(outerSize));
    allPixels.resize(outerSize);
    for (size_t i = 0; i < outerSize; ++i) {
      size_t innerSize;
      file.read(reinterpret_cast<char*>(&innerSize), sizeof(innerSize));
      allPixels[i].resize(innerSize);
      file.read(reinterpret_cast<char*>(allPixels[i].data()), innerSize * sizeof(INO::PixelId));
    }
    file.close();
  }

  void readPixelsFromAllFiles(const std::string& directory, std::vector<std::vector<std::vector<INO::PixelId>>>& allData) {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        std::vector<std::vector<INO::PixelId>> allPixels;
        readPixelsFromFile(entry.path().string(), allPixels);
        allData.push_back(allPixels);
      }
    }
  }
}
