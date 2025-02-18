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
      "Start REAL, End REAL, Module INTEGER, Row INTEGER, Column INTEGER, "
      "Layer INTEGER, Side INTEGER, Strip INTEGER, Value REAL, "
      "PRIMARY KEY (Start, End, Module, Row, Column, Layer, Side, Strip));";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
      std::cerr << "Error creating table: " << errMsg << std::endl;
      sqlite3_free(errMsg);
    }
  }
  {
    const char* sql = "CREATE TABLE IF NOT EXISTS StripPositionCorrection ("
      "Start REAL, End REAL, Module INTEGER, Row INTEGER, Column INTEGER, "
      "Layer INTEGER, Side INTEGER, Strip INTEGER, Position INTEGER, Value REAL, "
      "PRIMARY KEY (Start, End, Module, Row, Column, Layer, Side, Strip, Position));";
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

// void INOCalibrationManager::setStripTimeDelay(const StripId& stripId, double start, double end, double value) {
//   std::string sql = "INSERT INTO StripTimeDelay (Start, End, Module, Row, Column, Layer, Side, Strip, Value) "
//                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?) "
//                     "ON CONFLICT(Start, End, Module, Row, Column, Layer, Side, Strip) DO UPDATE SET Value=excluded.Value;";
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
//     sqlite3_bind_double(stmt, 9, value);
//     if (sqlite3_step(stmt) != SQLITE_DONE)
//       std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
//     sqlite3_finalize(stmt);
//   } else {
//     std::cerr << "SQL error in setStripTimeDelay: " << sqlite3_errmsg(db) << std::endl;
//   }
// }

double INOCalibrationManager::getStripTimeDelay(const StripId& stripId, double time) const {
  std::string sql = "SELECT Value FROM StripTimeDelay WHERE "
                    "Start <= ? AND End > ? AND Module=? AND Row=? AND Column=? "
                    "AND Layer=? AND Side=? AND Strip=? ORDER BY Start DESC LIMIT 1;";
  sqlite3_stmt* stmt;
  double value = 0.0;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, time);
    sqlite3_bind_double(stmt, 2, time);
    sqlite3_bind_int(stmt, 3, stripId.module);
    sqlite3_bind_int(stmt, 4, stripId.row);
    sqlite3_bind_int(stmt, 5, stripId.column);
    sqlite3_bind_int(stmt, 6, stripId.layer);
    sqlite3_bind_int(stmt, 7, stripId.side);
    sqlite3_bind_int(stmt, 8, stripId.strip);
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

void INOCalibrationManager::getLayerPosition(const StripId& stripId, const double& pos, TVector3 position, TVector3 orientation) const {
  const char* sql = "SELECT position_x, position_y, position_z, orientation_x, orientation_y, orientation_z "
    "FROM Position WHERE Module =? AND Row =? AND Column =? AND Layer = ? AND detector_type_x = ? AND detector_type_y = ?;";
  sqlite3_stmt* stmt;
  // Prepare the statement
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {

    // Bind parameters to the SQL query
    sqlite3_bind_int(stmt, 1, stripId.module);
    sqlite3_bind_int(stmt, 2, stripId.row);
    sqlite3_bind_int(stmt, 3, stripId.column);
    sqlite3_bind_int(stmt, 4, stripId.layer);
    sqlite3_bind_int(stmt, 5, stripId.side ? int(pos / 32.) : int(stripId.strip / 32.));
    sqlite3_bind_int(stmt, 6, !stripId.side ? int(pos / 32.) : int(stripId.strip / 32.));

    // Execute the query and retrieve the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      double position_x = sqlite3_column_double(stmt, 0);
      double position_y = sqlite3_column_double(stmt, 1);
      double position_z = sqlite3_column_double(stmt, 2);
      double orientation_x = sqlite3_column_double(stmt, 3);
      double orientation_y = sqlite3_column_double(stmt, 4);
      double orientation_z = sqlite3_column_double(stmt, 5);

      // // Output the retrieved values
      // std::cout << "Position: (" << position_x << ", " << position_y << ", " << position_z << ")\n";
      // std::cout << "Orientation: (" << orientation_x << ", " << orientation_y << ", " << orientation_z << ")\n";
    }
    sqlite3_finalize(stmt);
  }

  // Cleanup
}
