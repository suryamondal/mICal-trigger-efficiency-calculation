
#include <string>

namespace INO {

  std::string getStripName(const StripId& stripId) {
    return "m" + std::to_string(stripId.module) +
      "_r" + std::to_string(stripId.row) +
      "_c" + std::to_string(stripId.column) +
      "_l" + std::to_string(stripId.layer) +
      "_" + (stripId.side ? "x" : "y") +
      "_s" + std::to_string(stripId.strip);
  };
}
  
