#pragma once

#include <memory>
#include <map>

namespace INO {
  
  struct LayerId {
    int module;
    int row;
    int column;
    int layer;
    // Define operator< for map key comparison
    bool operator<(const LayerId& other) const {
      return std::tie(module, row, column, layer) < 
        std::tie(other.module, other.row, other.column, other.layer);
    }
  };

  struct TDCId {
    int module;
    int row;
    int column;
    int layer;
    int side;
    int tdc;
    // Define operator< for map key comparison
    bool operator<(const TDCId& other) const {
      return std::tie(module, row, column, layer, side, tdc) < 
        std::tie(other.module, other.row, other.column, other.layer, other.side, other.tdc);
    }
  };

  struct SideId {
    int module;
    int row;
    int column;
    int layer;
    int side;
    // Define operator< for map key comparison
    bool operator<(const SideId& other) const {
      return std::tie(module, row, column, layer, side) < 
        std::tie(other.module, other.row, other.column, other.layer, other.side);
    }
  };

  struct StripId {
    int module;
    int row;
    int column;
    int layer;
    int side;
    int strip;
    // Define operator< for map key comparison
    bool operator<(const StripId& other) const {
      return std::tie(module, row, column, layer, side, strip) < 
        std::tie(other.module, other.row, other.column, other.layer, other.side, other.strip);
    }
  };

  class INOCalibrationManager {
  public:
    INOCalibrationManager() {}
    // Get the singleton instance
    static std::shared_ptr<INOCalibrationManager> getInstance();

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

  private:
    static std::shared_ptr<INOCalibrationManager> instance;

    std::map<StripId, double> timeCalibration;
  };

} // namespace INO
