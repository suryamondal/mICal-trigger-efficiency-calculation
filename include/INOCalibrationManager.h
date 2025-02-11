#pragma once

#include <memory>
#include <map>
#include <string>

#include "INOStructs.h"

namespace INO {

  class INOCalibrationManager {
  public:
    INOCalibrationManager() {}
    // Get the singleton instance
    static INOCalibrationManager& getInstance();

    // Setter for time calibration
    void setTimeCalibration(const StripId& stripId, double time) {
      timeCalibration[stripId] = time;
    }

    // Getter for time calibration
    double getTimeCalibration(const StripId& stripId) const {
      auto it = timeCalibration.find(stripId);
      if (it != timeCalibration.end())
        return it->second;
      return 0;
    }

    void loadTimeCalibration(const std::string& filename);
    void writeTimeCalibration(const std::string& filename);

  private:
    std::map<StripId, double> timeCalibration;
  };

} // namespace INO
