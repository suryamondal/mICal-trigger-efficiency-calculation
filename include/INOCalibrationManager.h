#pragma once

#include <sqlite3.h>
#include <string>
#include <iostream>

#include "TVector3.h"

#include "INOStructs.h"

namespace INO {

  class INOCalibrationManager {
  public:
    static INOCalibrationManager& getInstance();

    // void setStripTimeOffset(const StripId& stripId, double start, double end, double value);
    double getStripTimeDelay(const StripId& stripId, double time) const;

    // void setStripPositionCorrection(const StripId& stripId, int position, double start, double end, double value);
    void getLayerPosition(const StripId& stripId, const double& pos, TVector3 position, TVector3 orientation) const;

  private:
    INOCalibrationManager();
    ~INOCalibrationManager();
    void initializeDatabase();

    sqlite3* db;
  };

} // namespace INO
