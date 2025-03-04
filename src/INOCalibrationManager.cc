#include "INOCalibrationManager.h"
#include <sstream>
#include <fstream>
#include <vector>

using namespace INO;

INOCalibrationManager::INOCalibrationManager() {
  if (sqlite3_open("calibration.db", &db) != SQLITE_OK)
    std::cerr << "Error opening database!" << std::endl;
  else
    initializeDatabase();
}

INOCalibrationManager::~INOCalibrationManager() {
  if (db) sqlite3_close(db);
}

void INOCalibrationManager::initializeDatabase() {
  {
    const char* sql = "CREATE TABLE IF NOT EXISTS StripTimeDelay ("
      "Module INTEGER, Row INTEGER, Column INTEGER, "
      "Layer INTEGER, Side INTEGER, Strip INTEGER, Value REAL, "
      "PRIMARY KEY (Module, Row, Column, Layer, Side, Strip));";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      std::cerr << "Error creating table: " << errMsg << std::endl;
      sqlite3_free(errMsg);
    }
  }
  {
    const char* sql = "CREATE TABLE IF NOT EXISTS RPCPosition ("
      "Module INTEGER, Row INTEGER, Column INTEGER, Layer INTEGER, "
      "detector_zone_x INTEGER, detector_zone_y INTEGER, "
      "position_x REAL, position_y REAL, position_z REAL, orientation_x REAL, orientation_y REAL, orientation_z REAL, "
      "PRIMARY KEY (Module, Row, Column, Layer, detector_zone_x, detector_zone_y));";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      std::cerr << "Error creating table: " << errMsg << std::endl;
      sqlite3_free(errMsg);
    }
  }
  {
    const char* sql = "CREATE TABLE IF NOT EXISTS StripletPosition ("
      "Module INTEGER, Row INTEGER, Column INTEGER, "
      "Layer INTEGER, Side INTEGER, Strip INTEGER, Zone INTEGER, "
      "position_x REAL, position_y REAL, position_z REAL, orientation_x REAL, orientation_y REAL, orientation_z REAL, "
      "PRIMARY KEY (Module, Row, Column, Layer, Side, Strip, Zone));";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      std::cerr << "Error creating table: " << errMsg << std::endl;
      sqlite3_free(errMsg);
    }
  }
}

INOCalibrationManager& INOCalibrationManager::getInstance() {
  static INOCalibrationManager instance;
  return instance;
}

void INOCalibrationManager::setStripTimeDelay(const StripId& stripId, double value) {
  sqlite3_busy_timeout(db, 5000);
  std::string sql = "INSERT OR REPLACE INTO StripTimeDelay (Module, Row, Column, Layer, Side, Strip, Value) "
    "VALUES (?, ?, ?, ?, ?, ?, ?);";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, stripId.module);
    sqlite3_bind_int(stmt, 2, stripId.row);
    sqlite3_bind_int(stmt, 3, stripId.column);
    sqlite3_bind_int(stmt, 4, stripId.layer);
    sqlite3_bind_int(stmt, 5, stripId.side);
    sqlite3_bind_int(stmt, 6, stripId.strip);
    sqlite3_bind_double(stmt, 7, value);
    if (sqlite3_step(stmt) != SQLITE_DONE)
      std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in setStripTimeDelay: " << sqlite3_errmsg(db) << std::endl;
  }
}

double INOCalibrationManager::getStripTimeDelay(const StripId& stripId) const {
  sqlite3_busy_timeout(db, 5000);
  std::string sql = "SELECT Value FROM StripTimeDelay WHERE "
    "Module=? AND Row=? AND Column=? "
    "AND Layer=? AND Side=? AND Strip=?;";
  sqlite3_stmt* stmt;
  double value = -265;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, stripId.module);
    sqlite3_bind_int(stmt, 2, stripId.row);
    sqlite3_bind_int(stmt, 3, stripId.column);
    sqlite3_bind_int(stmt, 4, stripId.layer);
    sqlite3_bind_int(stmt, 5, stripId.side);
    sqlite3_bind_int(stmt, 6, stripId.strip);
    if (sqlite3_step(stmt) == SQLITE_ROW)
      value = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in getStripTimeDelay: " << sqlite3_errmsg(db) << std::endl;
  }
  return value;
}

// void INOCalibrationManager::setStripPositionCorrection(const StripId& stripId, int position, double start, double end, double value) {
//   std::string sql = "INSERT INTO StripPositionCorrection (Start, End, Module, Row, Column, Layer, Side, Strip, Position, Value) "
//                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
//                     "ON CONFLICT(Start, End, Module, Row, Column, Layer, Side, Strip, Position) DO UPDATE SET Value=excluded.Value;";
//   sqlite3_stmt* stmt;
//   if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
//     sqlite3_bind_double(stmt, 1, start);
//     sqlite3_bind_double(stmt, 2, end);
//     sqlite3_bind_int(stmt, 3, stripId.module);
//     sqlite3_bind_int(stmt, 4, stripId.row);
//     sqlite3_bind_int(stmt, 5, stripId.column);
//     sqlite3_bind_int(stmt, 6, stripId.layer);
//     sqlite3_bind_int(stmt, 7, stripId.side);
//     sqlite3_bind_int(stmt, 8, stripId.strip);
//     sqlite3_bind_int(stmt, 9, position);
//     sqlite3_bind_double(stmt, 10, value);
//     if (sqlite3_step(stmt) != SQLITE_DONE)
//       std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
//     sqlite3_finalize(stmt);
//   } else {
//     std::cerr << "SQL error in setStripPositionCorrection: " << sqlite3_errmsg(db) << std::endl;
//   }
// }


void INOCalibrationManager::setLayerPosition(const LayerZoneId& layerZoneId,
                                             const TVector3& position, const TVector3& orientation) {
  sqlite3_busy_timeout(db, 5000);
  std::string sql = "INSERT INTO RPCPosition (Module, Row, Column, Layer, "
    "detector_zone_x, detector_zone_y, "
    "position_x, position_y, position_z, orientation_x, orientation_y, orientation_z) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, layerZoneId.module);
    sqlite3_bind_int(stmt, 2, layerZoneId.row);
    sqlite3_bind_int(stmt, 3, layerZoneId.column);
    sqlite3_bind_int(stmt, 4, layerZoneId.layer);
    sqlite3_bind_int(stmt, 5, layerZoneId.zone[0]);
    sqlite3_bind_int(stmt, 6, layerZoneId.zone[1]);
    sqlite3_bind_double(stmt, 7, position.X());
    sqlite3_bind_double(stmt, 8, position.Y());
    sqlite3_bind_double(stmt, 9, position.Z());
    sqlite3_bind_double(stmt, 10, orientation.X());
    sqlite3_bind_double(stmt, 11, orientation.Y());
    sqlite3_bind_double(stmt, 12, orientation.Z());
    if (sqlite3_step(stmt) != SQLITE_DONE)
      std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in setStripTimeDelay: " << sqlite3_errmsg(db) << std::endl;
  }
}


void INOCalibrationManager::getLayerPosition(const LayerZoneId& layerZoneId,
                                             TVector3& position, TVector3& orientation) const {
  const char* sql = "SELECT position_x, position_y, position_z, orientation_x, orientation_y, orientation_z "
    "FROM RPCPosition WHERE Module =? AND Row =? AND Column =? AND Layer = ? AND detector_zone_x = ? AND detector_zone_y = ?;";
  sqlite3_stmt* stmt;
  // Prepare the statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    // Bind parameters to the SQL query
    sqlite3_bind_int(stmt, 1, layerZoneId.module);
    sqlite3_bind_int(stmt, 2, layerZoneId.row);
    sqlite3_bind_int(stmt, 3, layerZoneId.column);
    sqlite3_bind_int(stmt, 4, layerZoneId.layer);
    sqlite3_bind_int(stmt, 5, layerZoneId.zone[0]);
    sqlite3_bind_int(stmt, 6, layerZoneId.zone[1]);
    // Execute the query and retrieve the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      position.SetX(sqlite3_column_double(stmt, 0));
      position.SetY(sqlite3_column_double(stmt, 1));
      position.SetZ(sqlite3_column_double(stmt, 2));
      orientation.SetX(sqlite3_column_double(stmt, 3));
      orientation.SetY(sqlite3_column_double(stmt, 4));
      orientation.SetZ(sqlite3_column_double(stmt, 5));
      // std::cout << "Position: (" << position.X() << ", " << position.Y() << ", " << position.Z() << ")\n";
      // std::cout << "Orientation: (" << orientation.X() << ", " << orientation.Y() << ", " << orientation.Z() << ")\n";
    }
    sqlite3_finalize(stmt);
  }
}


void INOCalibrationManager::setStripletPosition(const StripletId& stripletId,
                                                const TVector3& position, const TVector3& orientation) {
  sqlite3_busy_timeout(db, 5000);
  std::string sql = "INSERT INTO StripletPosition (Module, Row, Column, Layer, Zone, "
    "position_x, position_y, position_z, orientation_x, orientation_y, orientation_z) "
    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, stripletId.module);
    sqlite3_bind_int(stmt, 2, stripletId.row);
    sqlite3_bind_int(stmt, 3, stripletId.column);
    sqlite3_bind_int(stmt, 4, stripletId.layer);
    sqlite3_bind_int(stmt, 5, stripletId.side);
    sqlite3_bind_int(stmt, 6, stripletId.strip);
    sqlite3_bind_double(stmt, 7, position.X());
    sqlite3_bind_double(stmt, 8, position.Y());
    sqlite3_bind_double(stmt, 9, position.Z());
    sqlite3_bind_double(stmt, 10, orientation.X());
    sqlite3_bind_double(stmt, 11, orientation.Y());
    sqlite3_bind_double(stmt, 12, orientation.Z());
    if (sqlite3_step(stmt) != SQLITE_DONE)
      std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in setStripTimeDelay: " << sqlite3_errmsg(db) << std::endl;
  }
}


void INOCalibrationManager::getStripletPosition(const StripletId& stripletId,
                                                TVector3& position, TVector3& orientation) const {
  const char* sql = "SELECT position_x, position_y, position_z, orientation_x, orientation_y, orientation_z "
    "FROM StripletPosition WHERE Module =? AND Row =? AND Column =? AND Layer = ? AND Strip = ? AND Zone = ?;";
  sqlite3_stmt* stmt;
  // Prepare the statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    // Bind parameters to the SQL query
    sqlite3_bind_int(stmt, 1, stripletId.module);
    sqlite3_bind_int(stmt, 2, stripletId.row);
    sqlite3_bind_int(stmt, 3, stripletId.column);
    sqlite3_bind_int(stmt, 4, stripletId.layer);
    sqlite3_bind_int(stmt, 5, stripletId.strip);
    sqlite3_bind_int(stmt, 6, stripletId.zone);
    // Execute the query and retrieve the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      position.SetX(sqlite3_column_double(stmt, 0));
      position.SetY(sqlite3_column_double(stmt, 1));
      position.SetZ(sqlite3_column_double(stmt, 2));
      orientation.SetX(sqlite3_column_double(stmt, 3));
      orientation.SetY(sqlite3_column_double(stmt, 4));
      orientation.SetZ(sqlite3_column_double(stmt, 5));
      // std::cout << "Position: (" << position.X() << ", " << position.Y() << ", " << position.Z() << ")\n";
      // std::cout << "Orientation: (" << orientation.X() << ", " << orientation.Y() << ", " << orientation.Z() << ")\n";
    }
    sqlite3_finalize(stmt);
  }
}
