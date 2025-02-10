#pragma once

#include <vector>
#include <map>
#include <tuple>
#include <limits>
#include <optional>
#include <iostream>
#include <cmath>

#include "INOCalibrationManager.h"

namespace INO {

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
    INOEvent();

    void addHit(const StripId& stripId);
    bool hasHit(const StripId& stripId) const;
    void removeHit(const StripId& stripId);

    // Method to get all hits
    std::vector<const Hit*> getHits() const;
    // Method to get raw leading time of a hit
    double getRawLeadingTime(const StripId& stripId) const;
    std::vector<double> getRawLeadingTimes(const StripId& stripId) const;
    // Method to get all leading TDC values
    std::vector<double> getCalibratedLeadingTimes(const StripId& stripId) const;

    // Method to get tracked leading time of a hit
    double getTrackedLeadingTime(const StripId& stripId) const;
    // Method to get aligned position of a hit
    double getAlignedPosition(const StripId& stripId) const;

    // Add TDC time for leading or trailing edge
    void addTDC(const TDCId& tdcId, double time, bool isTrailing);
    // Method to get all leading TDC values
    std::vector<double> getLeadingTDCs() const;
    // setter for event time 
    void setEventTime(double time);
    // Getter for event time 
    double getEventTime() const;

    // Setter to compute and update lowest and highest calibrated leading times
    void updateCalibratedLeadingTimeBounds();
    // Getter for lowest calibrated leading time
    double getLowestCalibratedLeadingTime();
    // Getter for highest calibrated leading time
    double getHighestCalibratedLeadingTime();

    /** Get ID of the time-group.
     * @return time-group ID
     */
    std::vector<int> getTimeGroupId(const StripId& stripId) const;
    /** Get time-group parameters.
     * @return time-group parameters (integral, center, sigma)
     */
    std::vector<std::tuple<float, float, float>> getTimeGroupInfo(const StripId& stripId) const;
    /** Set ID of the time-group.
     * @return reference to time-group ID
     */
    std::vector<int>& setTimeGroupId(const StripId& stripId);
    /** Set time-group parameters.
     * @return reference to the time-group parameters (integral, center, sigma)
     */
    std::vector<std::tuple<float, float, float>>& setTimeGroupInfo(const StripId& stripId);

    int getEntries() const;

  private:
    std::map<StripId, Hit> rawHits;
    std::map<TDCId, std::vector<double>> rawTDCs[2]; /* leading and trailing */
    double eventTime;
    double lowestCalibratedLeadingTime;
    double highestCalibratedLeadingTime;
  };

} // namespace INO
