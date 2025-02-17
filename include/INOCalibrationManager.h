#pragma once

#include <sqlite3.h>
#include <string>
#include <iostream>

#include "INOStructs.h"

namespace INO {

  class INOCalibrationManager {
  public:
    static INOCalibrationManager& getInstance();

    void setStripTimeOffset(const StripId& stripId, double start, double end, double value);
    double getStripTimeOffset(const StripId& stripId, double time) const;

    void setStripPositionCorrection(const StripId& stripId, int position, double start, double end, double value);
    double getStripPositionCorrection(const StripId& stripId, int position, double time) const;

  private:
    INOCalibrationManager();
    ~INOCalibrationManager();
    void initializeDatabase();

    sqlite3* db;
  };

} // namespace INO
