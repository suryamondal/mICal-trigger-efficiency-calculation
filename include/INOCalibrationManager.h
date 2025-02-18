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

    double getStripTimeDelay(const StripId& stripId, double time) const;
    void getLayerPosition(const LayerId& layerId, const int& x, const int& y, TVector3& position, TVector3& orientation) const;

  private:
    INOCalibrationManager();
    ~INOCalibrationManager();
    void initializeDatabase();

    sqlite3* db;
  };

} // namespace INO
