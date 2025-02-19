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
}

INOCalibrationManager& INOCalibrationManager::getInstance() {
  static INOCalibrationManager instance;
  return instance;
}

void INOCalibrationManager::setStripTimeDelay(const StripId& stripId, double value) {
  std::string sql = "INSERT INTO StripTimeDelay (Module, Row, Column, Layer, Side, Strip, Value) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?) "
                    "ON CONFLICT(Module, Row, Column, Layer, Side, Strip) DO UPDATE SET Value=excluded.Value;";
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
  std::string sql = "SELECT Value FROM StripTimeDelay WHERE "
                    "Module=? AND Row=? AND Column=? "
                    "AND Layer=? AND Side=? AND Strip=?;";
  sqlite3_stmt* stmt;
  double value = 0.0;
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

void INOCalibrationManager::getLayerPosition(const LayerId& layerId, const int& x, const int& y,
					     TVector3& position, TVector3& orientation) const {
  const char* sql = "SELECT position_x, position_y, position_z, orientation_x, orientation_y, orientation_z "
    "FROM Position WHERE Module =? AND Row =? AND Column =? AND Layer = ? AND detector_type_x = ? AND detector_type_y = ?;";
  sqlite3_stmt* stmt;
  // Prepare the statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {

    // Bind parameters to the SQL query
    sqlite3_bind_int(stmt, 1, layerId.module);
    sqlite3_bind_int(stmt, 2, layerId.row);
    sqlite3_bind_int(stmt, 3, layerId.column);
    sqlite3_bind_int(stmt, 4, layerId.layer);
    sqlite3_bind_int(stmt, 5, x);
    sqlite3_bind_int(stmt, 6, y);

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

  // Cleanup
}
