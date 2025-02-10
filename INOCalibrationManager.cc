#include "INOCalibrationManager.h"

namespace INO {
  std::shared_ptr<INOCalibrationManager> INOCalibrationManager::instance = nullptr;

  std::shared_ptr<INOCalibrationManager> INOCalibrationManager::getInstance() {
    if (!instance)
      instance = std::make_shared<INOCalibrationManager>();
    return instance;
  }
}
