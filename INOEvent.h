#pragma once

#include <vector>
#include <map>
#include <tuple>
#include <limits>
#include <optional>
#include <iostream>

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

  struct HitId {
    int module;
    int row;
    int column;
    int layer;
    int side;
    int strip;
    // Define operator< for map key comparison
    bool operator<(const HitId& other) const {
      return std::tie(module, row, column, layer, side, strip) < 
	std::tie(other.module, other.row, other.column, other.layer, other.side, other.strip);
    }
  };

  struct Hit {
    HitId hitId;
    std::vector<double> rawTimes[2]; // leading and trailing
    std::vector<double> calibratedTimes[2];
    double trackedCalibratedTime[2];
    double rawPosition;
    double alignedPosition;
  };

  class INOEvent {
  public:
    INOEvent() : eventTime(std::numeric_limits<double>::quiet_NaN()),
		 shiftTDCTime(std::numeric_limits<double>::quiet_NaN()) {
      rawHits.clear();
      rawTDCs[0].clear();
      rawTDCs[1].clear();
    }

    void addHit(const HitId& hitId) {
      Hit rawHit;
      rawHit.hitId = hitId;
      rawHit.rawPosition = hitId.strip + 0.5;
      for (int timeType = 0; timeType < 2; timeType++) {
	/* 0 for leading, 1 for trailing */
	/* 8 TDCs per side */
	TDCId tdcId = {hitId.module, hitId.row, hitId.column,
		       hitId.layer, hitId.side, hitId.strip % 8};
	if (rawTDCs[timeType].count(tdcId))
	  rawHit.rawTimes[timeType] = rawTDCs[timeType][tdcId];
      }
      rawHits[hitId] = rawHit; 
    }

    bool hasHit(const HitId& hitId) const {
      return rawHits.find(hitId) != rawHits.end(); 
    }

    void removeHit(const HitId& hitId) { 
      rawHits.erase(hitId); 
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

    // Method to get tracked leading time of a hit
    double getTrackedLeadingTime(const HitId& hitId) const {
      auto it = rawHits.find(hitId);
      if (it != rawHits.end())
	return it->second.trackedCalibratedTime[0];
      return std::numeric_limits<double>::quiet_NaN();
    }

    // Method to get aligned position of a hit
    double getAlignedPosition(const HitId& hitId) const {
      auto it = rawHits.find(hitId);
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

    // Setter for shift TDC time
    void setShiftTDCTime(double time) {
      shiftTDCTime = time;
    }

    // Getter for shift TDC time
    double getShiftTDCTime() const {
      return shiftTDCTime;
    }

  private:
    std::map<HitId, Hit> rawHits;
    std::map<TDCId, std::vector<double>> rawTDCs[2]; /* 8 TDCs per side, both leading and trailing */
    double eventTime;
    double shiftTDCTime;
  };

} // namespace INO
