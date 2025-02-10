#include <iostream>
#include <set>
#include <vector>
#include <cmath>

struct Bin {
  double center;
  int count;

  // Comparator for sorting bins by count (descending)
  bool operator<(const Bin& other) const {
    return count > other.count;  // Higher count bins come first
  }
};

class DynamicHistogram {
 private:
  std::set<Bin> bins;
  double binWidth;
  int maxBins;

 public:
 DynamicHistogram(double width = 1, int maxBins = 100) : binWidth(width), maxBins(maxBins) {}

  void fillValue(double value) {
    // Try to find an existing bin
    for (auto& bin : bins)
      if (std::fabs(bin.center - value) <= binWidth / 2.0) {
        Bin updated = bin;
        updated.count++;
        bins.erase(bin);
        bins.insert(updated);
        return;
      }

    // If no suitable bin found, create a new one
    if (bins.size() < maxBins)
      bins.insert({value, 1});
    if (bins.size() >= maxBins) {
      // Remove the least populated bin (last element in sorted order)
      auto it = bins.rbegin();
      bins.erase(--it.base());
    }
  }

  Bin getPeak() {
    return *bins.begin();  // First element (largest count)
  }

  void printBins() {
    for (const auto& bin : bins) {
      std::cout << "Bin: " << bin.center << ", Count: " << bin.count << std::endl;
    }
  }
};
