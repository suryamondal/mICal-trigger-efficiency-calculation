#pragma once

#include <vector>
#include <map>
#include <tuple>
#include <limits>
#include <optional>
#include <iostream>
#include <cmath>

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

  struct Hit {
    StripId stripId;
    std::vector<double> rawTimes[2]; // leading and trailing
    std::vector<double> calibratedTimes[2];
    double trackedCalibratedTime[2];
    double rawPosition;
    double alignedPosition;
  };

  class INOEvent {
  public:
    INOEvent() : eventTime(std::numeric_limits<double>::quiet_NaN()),
                 lowestCalibratedLeadingTime(std::numeric_limits<double>::quiet_NaN()),
                 highestCalibratedLeadingTime(std::numeric_limits<double>::quiet_NaN())
    {
      rawHits.clear();
      rawTDCs[0].clear();
      rawTDCs[1].clear();
    }

    void addHit(const StripId& stripId) {
      Hit rawHit;
      rawHit.stripId = stripId;
      rawHit.rawPosition = stripId.strip + 0.5;
      for (int timeType = 0; timeType < 2; timeType++) {
        /* 0 for leading, 1 for trailing */
        /* 8 TDCs per side */
        TDCId tdcId = {stripId.module, stripId.row, stripId.column,
                       stripId.layer, stripId.side, stripId.strip % 8};
        if (rawTDCs[timeType].count(tdcId))
          rawHit.rawTimes[timeType] = rawTDCs[timeType][tdcId];
      }
      rawHits[stripId] = rawHit; 
    }

    bool hasHit(const StripId& stripId) const {
      return rawHits.count(stripId);
    }

    void removeHit(const StripId& stripId) { 
      rawHits.erase(stripId); 
    }

    // Method to get all hits
    std::vector<const Hit*> getHits() const {
      std::vector<const Hit*> hits;
      for (const auto& entry : rawHits) {
        hits.push_back(&entry.second);
      }
      return hits;
    }

    /* std::vector<Hit> getHits() const {  */
    /*   std::vector<Hit> hits; */
    /*   for (const auto& entry : rawHits) { */
    /* 	hits.push_back(entry.second); */
    /*   } */
    /*   return hits; */
    /* } */

    // Method to get raw leading time of a hit
    double getRawLeadingTime(const StripId& stripId) const {
      if (rawHits.count(stripId)) {
        auto it = rawHits.find(stripId);
        if (int(it->second.rawTimes[0].size()))
          return it->second.rawTimes[0][0];
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Method to get tracked leading time of a hit
    double getTrackedLeadingTime(const StripId& stripId) const {
      if (rawHits.count(stripId)) {
        auto it = rawHits.find(stripId);
        return it->second.trackedCalibratedTime[0];
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Method to get aligned position of a hit
    double getAlignedPosition(const StripId& stripId) const {
      if (rawHits.count(stripId)) {
        auto it = rawHits.find(stripId);
        return it->second.alignedPosition;
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Add TDC time for leading or trailing edge
    void addTDC(const TDCId& tdcId, double time, bool isTrailing) {
      int index = isTrailing ? 1 : 0;
      rawTDCs[index][tdcId].push_back(time);
    }

    // Method to get all leading TDC values
    std::vector<double> getLeadingTDCs() const {
      std::vector<double> leadingTDCs;
      for (const auto& entry : rawTDCs[0]) { // 0 = leading times
        const std::vector<double>& times = entry.second;
        leadingTDCs.insert(leadingTDCs.end(), times.begin(), times.end());
      }
      return leadingTDCs;
    }

    // setter for event time 
    void setEventTime(double time) {
      eventTime = time;
    }

    // Getter for event time 
    double getEventTime() const {
      return eventTime;
    }

    // Setter to compute and update lowest and highest calibrated leading times
    void updateCalibratedLeadingTimeBounds() {
      lowestCalibratedLeadingTime = std::numeric_limits<double>::infinity();
      highestCalibratedLeadingTime = -std::numeric_limits<double>::infinity();
      for (const auto& entry : rawHits) {
        const StripId& stripId = entry.first;
        const Hit& hit = entry.second;
        if (hit.rawTimes[0].empty()) continue;
        double calibration = getTimeCalibration(stripId, 0);
        for (double rawTime : hit.rawTimes[0]) {
          double adjustedTime = rawTime - calibration;
          if (adjustedTime < lowestCalibratedLeadingTime)
            lowestCalibratedLeadingTime = adjustedTime;
          if (adjustedTime > highestCalibratedLeadingTime)
            highestCalibratedLeadingTime = adjustedTime;
        }
      }
      // Handle case when no valid times are found
      if (lowestCalibratedLeadingTime == std::numeric_limits<double>::infinity()) {
        lowestCalibratedLeadingTime = std::numeric_limits<double>::quiet_NaN();
      }
      if (highestCalibratedLeadingTime == -std::numeric_limits<double>::infinity()) {
        highestCalibratedLeadingTime = std::numeric_limits<double>::quiet_NaN();
      }
    }

    // Getter for lowest calibrated leading time
    double getLowestCalibratedLeadingTime() {
      if (std::isnan(lowestCalibratedLeadingTime))
        updateCalibratedLeadingTimeBounds();
      return lowestCalibratedLeadingTime;
    }

    // Getter for highest calibrated leading time
    double getHighestCalibratedLeadingTime() {
      if (std::isnan(highestCalibratedLeadingTime))
        updateCalibratedLeadingTimeBounds();
      return highestCalibratedLeadingTime;
    }

    // Setter for time calibration
    void setTimeCalibration(const StripId& stripId, double time) {
      timeCalibration[stripId] = time;
    }

    // Setter for time calibration
    double getTimeCalibration(const StripId& stripId, double time) const {
      if (timeCalibration.count(stripId)) {
        auto it = timeCalibration.find(stripId);
        return it->second;
      }
      return 0;
    }

  private:
    std::map<StripId, Hit> rawHits;
    std::map<TDCId, std::vector<double>> rawTDCs[2]; /* 8 TDCs per side, both leading and trailing */
    std::map<StripId, double> timeCalibration;
    double eventTime;
    double lowestCalibratedLeadingTime;
    double highestCalibratedLeadingTime;
  };

} // namespace INO
