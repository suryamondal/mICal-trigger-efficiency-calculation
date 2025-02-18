#include "INOEvent.h"

namespace INO {

  INOEvent::INOEvent() 
    : eventTime(std::numeric_limits<double>::quiet_NaN()),
      lowestCalibratedLeadingTime(std::numeric_limits<double>::quiet_NaN()),
      highestCalibratedLeadingTime(std::numeric_limits<double>::quiet_NaN()) {
    rawHits.clear();
    rawTDCs[0].clear();
    rawTDCs[1].clear();
  }

  void INOEvent::addHit(const StripId& stripId) {
    INOCalibrationManager& inoCalibrationManager = INOCalibrationManager::getInstance();
    Hit rawHit;
    rawHit.stripId = stripId;
    rawHit.rawPosition = stripId.strip + 0.5;
    for (int timeType = 0; timeType < 2; timeType++) {
      TDCId tdcId = {stripId.module, stripId.row, stripId.column,
                     stripId.layer, stripId.side, stripId.strip % 8};
      if (rawTDCs[timeType].count(tdcId))
        rawHit.rawTimes[timeType] = rawTDCs[timeType][tdcId];
      for (auto rawTime : rawHit.rawTimes[timeType])
        rawHit.calibratedTimes[timeType].push_back(rawTime - inoCalibrationManager.getStripTimeDelay(stripId, getEventTime()));
      // std::cout << getEventTime() << " " << inoCalibrationManager.getStripTimeOffset(stripId, getEventTime()) << std::endl;
    }
    rawHits[stripId] = rawHit; 
  }

  bool INOEvent::hasHit(const StripId& stripId) const {
    return rawHits.count(stripId);
  }

  void INOEvent::removeHit(const StripId& stripId) { 
    rawHits.erase(stripId); 
  }

  std::vector<const Hit*> INOEvent::getHits() const {
    std::vector<const Hit*> hits;
    for (const auto& entry : rawHits) {
      hits.push_back(&entry.second);
    }
    return hits;
  }

  double INOEvent::getRawLeadingTime(const StripId& stripId) const {
    auto it = rawHits.find(stripId);
    if (it != rawHits.end() && !it->second.rawTimes[0].empty())
      return it->second.rawTimes[0][0];
    return std::numeric_limits<double>::quiet_NaN();
  }

  std::vector<double> INOEvent::getRawLeadingTimes(const StripId& stripId) const {
    auto it = rawHits.find(stripId); 
    if (it != rawHits.end())
      return it->second.rawTimes[0];
    return {};
  }

  std::vector<double> INOEvent::getCalibratedLeadingTimes(const StripId& stripId) const {
    auto it = rawHits.find(stripId); 
    if (it != rawHits.end())
      return it->second.calibratedTimes[0];
    return {};
  }

  double INOEvent::getTrackedLeadingTime(const StripId& stripId) const {
    auto it = rawHits.find(stripId);
    if (it != rawHits.end())
      return it->second.trackedCalibratedTime[0];
    return std::numeric_limits<double>::quiet_NaN();
  }

  double INOEvent::getAlignedPosition(const StripId& stripId) const {
    auto it = rawHits.find(stripId);
    if (it != rawHits.end())
      return it->second.alignedPosition;
    return std::numeric_limits<double>::quiet_NaN();
  }

  void INOEvent::addTDC(const TDCId& tdcId, double time, bool isTrailing) {
    int index = isTrailing ? 1 : 0;
    rawTDCs[index][tdcId].push_back(time);
  }

  std::vector<double> INOEvent::getLeadingTDCs() const {
    std::vector<double> leadingTDCs;
    for (const auto& entry : rawTDCs[0]) {
      leadingTDCs.insert(leadingTDCs.end(), entry.second.begin(), entry.second.end());
    }
    return leadingTDCs;
  }

  void INOEvent::setEventTime(double time) {
    eventTime = time;
  }

  double INOEvent::getEventTime() const {
    return eventTime;
  }

  void INOEvent::updateCalibratedLeadingTimeBounds() {
    lowestCalibratedLeadingTime = std::numeric_limits<double>::infinity();
    highestCalibratedLeadingTime = -std::numeric_limits<double>::infinity();
    for (const auto& entry : rawHits) {
      if (entry.second.calibratedTimes[0].empty()) continue;
      for (double calibratedTime : entry.second.calibratedTimes[0]) {
        lowestCalibratedLeadingTime = std::min(lowestCalibratedLeadingTime, calibratedTime);
        highestCalibratedLeadingTime = std::max(highestCalibratedLeadingTime, calibratedTime);
      }
    }
    if (lowestCalibratedLeadingTime == std::numeric_limits<double>::infinity())
      lowestCalibratedLeadingTime = std::numeric_limits<double>::quiet_NaN();  
    if (highestCalibratedLeadingTime == -std::numeric_limits<double>::infinity())
      highestCalibratedLeadingTime = std::numeric_limits<double>::quiet_NaN();
  }

  double INOEvent::getLowestCalibratedLeadingTime() {
    if (std::isnan(lowestCalibratedLeadingTime))
      updateCalibratedLeadingTimeBounds();
    return lowestCalibratedLeadingTime;
  }

  double INOEvent::getHighestCalibratedLeadingTime() {
    if (std::isnan(highestCalibratedLeadingTime))
      updateCalibratedLeadingTimeBounds();
    return highestCalibratedLeadingTime;
  }

  std::vector<int> INOEvent::getTimeGroupId(const StripId& stripId) const {
    return rawHits.at(stripId).m_timeGroupId;
  }

  std::vector<std::tuple<float, float, float>> INOEvent::getTimeGroupInfo(const StripId& stripId) const {
    return rawHits.at(stripId).m_timeGroupInfo;
  }

  std::vector<int>& INOEvent::setTimeGroupId(const StripId& stripId) {
    return rawHits[stripId].m_timeGroupId;
  }

  std::vector<std::tuple<float, float, float>>& INOEvent::setTimeGroupInfo(const StripId& stripId) {
    return rawHits[stripId].m_timeGroupInfo;
  }

  int INOEvent::getEntries() const {
    return rawHits.size();
  }

} // namespace INO
