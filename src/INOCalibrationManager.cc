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
    const char* sql = "CREATE TABLE IF NOT EXISTS ConstantStripTimeDelay ("
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

void INOCalibrationManager::setStripTimeOffset(const StripId& stripId, double start, double end, double value) {
  std::string sql = "INSERT INTO ConstantStripTimeDelay (Start, End, Module, Row, Column, Layer, Side, Strip, Value) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?) "
                    "ON CONFLICT(Start, End, Module, Row, Column, Layer, Side, Strip) DO UPDATE SET Value=excluded.Value;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, start);
    sqlite3_bind_double(stmt, 2, end);
    sqlite3_bind_int(stmt, 3, stripId.module);
    sqlite3_bind_int(stmt, 4, stripId.row);
    sqlite3_bind_int(stmt, 5, stripId.column);
    sqlite3_bind_int(stmt, 6, stripId.layer);
    sqlite3_bind_int(stmt, 7, stripId.side);
    sqlite3_bind_int(stmt, 8, stripId.strip);
    sqlite3_bind_double(stmt, 9, value);
    if (sqlite3_step(stmt) != SQLITE_DONE)
      std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in setStripTimeOffset: " << sqlite3_errmsg(db) << std::endl;
  }
}

double INOCalibrationManager::getStripTimeOffset(const StripId& stripId, double time) const {
  std::string sql = "SELECT Value FROM ConstantStripTimeDelay WHERE "
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
    std::cerr << "SQL error in getStripTimeOffset: " << sqlite3_errmsg(db) << std::endl;
  }
  return value;
}

void INOCalibrationManager::setStripPositionCorrection(const StripId& stripId, int position, double start, double end, double value) {
  std::string sql = "INSERT INTO StripPositionCorrection (Start, End, Module, Row, Column, Layer, Side, Strip, Position, Value) "
                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                    "ON CONFLICT(Start, End, Module, Row, Column, Layer, Side, Strip, Position) DO UPDATE SET Value=excluded.Value;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, start);
    sqlite3_bind_double(stmt, 2, end);
    sqlite3_bind_int(stmt, 3, stripId.module);
    sqlite3_bind_int(stmt, 4, stripId.row);
    sqlite3_bind_int(stmt, 5, stripId.column);
    sqlite3_bind_int(stmt, 6, stripId.layer);
    sqlite3_bind_int(stmt, 7, stripId.side);
    sqlite3_bind_int(stmt, 8, stripId.strip);
    sqlite3_bind_int(stmt, 9, position);
    sqlite3_bind_double(stmt, 10, value);
    if (sqlite3_step(stmt) != SQLITE_DONE)
      std::cerr << "Error inserting/updating calibration data: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in setStripPositionCorrection: " << sqlite3_errmsg(db) << std::endl;
  }
}

double INOCalibrationManager::getStripPositionCorrection(const StripId& stripId, int position, double time) const {
  std::string sql = "SELECT Value FROM StripPositionCorrection WHERE "
                    "Start <= ? AND End > ? AND Module=? AND Row=? AND Column=? "
                    "AND Layer=? AND Side=? AND Strip=? AND Position =? AND ORDER BY Start DESC LIMIT 1;";
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
    sqlite3_bind_int(stmt, 9, position);
    if (sqlite3_step(stmt) == SQLITE_ROW)
      value = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
  } else {
    std::cerr << "SQL error in getStripPositionCorrection: " << sqlite3_errmsg(db) << std::endl;
  }
  return value;
}
