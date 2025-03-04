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

    void setStripTimeDelay(const StripId& stripId, double time);
    double getStripTimeDelay(const StripId& stripId) const;
    void setLayerPosition(const LayerZoneId& layerZoneId, const TVector3& position, const TVector3& orientation);
    void getLayerPosition(const LayerZoneId& layerZoneId, TVector3& position, TVector3& orientation) const;
    void setStripletPosition(const StripletId& stripletId, const TVector3& position, const TVector3& orientation);
    void getStripletPosition(const StripletId& stripletId, TVector3& position, TVector3& orientation) const;

  private:
    INOCalibrationManager();
    ~INOCalibrationManager();
    void initializeDatabase();

    sqlite3* db;
  };

} // namespace INO
