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
    std::vector<int> m_timeGroupId;    /**< Grouping of clusters in time */
    std::vector<std::tuple<float, float, float>> m_timeGroupInfo; /**< TimeGroup Gaussian Parameters, (integral, center, sigma) */
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
        for (auto rawTime : rawHit.rawTimes[timeType])
          rawHit.calibratedTimes[timeType].push_back(item - getTimeCalibration(stripId));
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
      auto it = rawHits.find(stripId);
      if (it != rawHits.end()) {
        if (int(it->second.rawTimes[0].size()))
          return it->second.rawTimes[0][0];
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Method to get all leading TDC values
    std::vector<double> getCalibratedLeadingTimes(const StripId& stripId) const {
      auto it = rawHits.find(stripId); 
      if (it != rawHits.end())
        return it->second.calibratedTimes[0];
      return {};
    }

    // Method to get tracked leading time of a hit
    double getTrackedLeadingTime(const StripId& stripId) const {
      auto it = rawHits.find(stripId);
      if (it != rawHits.end())
        return it->second.trackedCalibratedTime[0];
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Method to get aligned position of a hit
    double getAlignedPosition(const StripId& stripId) const {
      auto it = rawHits.find(stripId);
      if (it != rawHits.end())
        return it->second.alignedPosition;
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
        double calibration = getTimeCalibration(stripId);
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

    // Getter for time calibration
    double getTimeCalibration(const StripId& stripId) const {
      auto it = timeCalibration.find(stripId);
      if (it != timeCalibration.end())
        return it->second;
      return 0;
    }

    /** Get ID of the time-group.
     * @return time-group ID
     */
    const std::vector<int>& getTimeGroupId(const StripId& stripId) const { return rawHits[stripId].m_timeGroupId; }

    /** Get time-group parameters.
     * @return time-group parameters (integral, center, sigma)
     */
    const std::vector<std::tuple<float, float, float>>& getTimeGroupInfo(const StripId& stripId) const { return rawHits[stripId].m_timeGroupInfo; }

    /** Set ID of the time-group.
     * @return reference to time-group ID
     */
    std::vector<int>& setTimeGroupId(const StripId& stripId) { return rawHits[stripId].m_timeGroupId; }

    /** Set time-group parameters.
     * @return reference to the time-group parameters (integral, center, sigma)
     */
    std::vector<std::tuple<float, float, float>>& setTimeGroupInfo(const StripId& stripId) { return rawHits[stripId].m_timeGroupInfo; }

    int getEntries() const { return rawHits.size(); }

  private:
    std::map<StripId, Hit> rawHits;
    std::map<TDCId, std::vector<double>> rawTDCs[2]; /* leading and trailing */
    std::map<StripId, double> timeCalibration;
    double eventTime;
    double lowestCalibratedLeadingTime;
    double highestCalibratedLeadingTime;
  };

} // namespace INO
