#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>

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
    // Define operator== for equality comparison
    bool operator==(const LayerId& other) const {
      return std::tie(module, row, column, layer) == 
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

  struct PixelId {
    int module;
    int row;
    int column;
    int layer;
    int strip[2];
    // Define operator< for map key comparison
    bool operator<(const PixelId& other) const {
      return std::tie(module, row, column, layer, strip[0], strip[1]) < 
        std::tie(other.module, other.row, other.column, other.layer, other.strip[0], other.strip[1]);
    }
  };
  
  struct Hit {
    StripId stripId;
    std::vector<double> rawTimes[2]; // leading and trailing
    std::vector<double> calibratedTimes[2];
    double trackedCalibratedTime[2];
    double rawPosition;
    double alignedPosition;
    std::vector<int> m_timeGroupId;    /**< Grouping of clusters in time */
    std::vector<std::tuple<float, float, float>> m_timeGroupInfo; /**< TimeGroup Gaussian Parameters, (integral, center, sigma) */
  };

} // namespace INO
