#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include <chrono>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <bitset>
#include <memory>
#include <csignal>
#include <algorithm>
#include <fstream>

#include "TTimeStamp.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TTree.h"
#include "TFile.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TObject.h"
#include "TRandom.h"
#include "TVector2.h"
#include "TVector3.h"
#include "TGraph.h"
#include "TMinuit.h"
#include "TF1.h"

#include "SNM.h"
#include "INOEvent.h"
#include "INOStorageManager.h"
#include "INOTimeGroupingModule.h"
// #include "DynamicHistogram.h"
#include "INOHelperFunctions.h"



// Function to compute something
double computeFunction(std::vector<INO::PixelId> allPixels) {
  return 1;
}

// Thread worker function
void worker(std::queue<std::vector<INO::PixelId>>& taskQueue, std::mutex& queueMutex,
            std::condition_variable& cv,
            std::vector<double>& results, std::mutex& resultsMutex, bool& done) {
  while (true) {
    std::vector<INO::PixelId> task;
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      cv.wait(lock, [&]() { return !taskQueue.empty() || done; });
      if (taskQueue.empty() && done) return; // Exit if all tasks are done
      task = taskQueue.front();
      taskQueue.pop();
    }
    double result = computeFunction(task);
    {
      std::lock_guard<std::mutex> lock(resultsMutex);
      results.push_back(result);
    }
  }
}

int main(int argc, char** argv) {

  // Start timer
  auto start = std::chrono::high_resolution_clock::now();

  // Number of worker threads
  unsigned int maxworker = std::max(std::thread::hardware_concurrency() - 1, 1u);
  std::cout << "Using " << maxworker << " worker threads.\n";

  // Task queue and synchronization primitives
  std::queue<std::vector<INO::PixelId>> taskQueue;
  std::mutex queueMutex, resultsMutex;
  std::condition_variable cv;
  std::vector<double> results;
  bool done = false;

  std::vector<std::vector<INO::PixelId>> allPixelsInEvents;
  readPixelsFromAllFiles(argv[1], allPixelsInEvents);
  // Generate tasks
  for (auto pixels : allPixelsInEvents) {
    taskQueue.push(pixels);
  }

  // Create worker threads
  std::vector<std::thread> workers;
  for (unsigned int i = 0; i < maxworker; i++) {
    workers.emplace_back(worker, std::ref(taskQueue), std::ref(queueMutex), std::ref(cv), 
                         std::ref(results), std::ref(resultsMutex), std::ref(done));
  }

  // Notify workers that tasks are available
  cv.notify_all();

  // Wait for all tasks to be processed
  while (true) {
    std::unique_lock<std::mutex> lock(queueMutex);
    if (taskQueue.empty()) {
      done = true;
      cv.notify_all();
      break;
    }
  }

  // Join all worker threads
  for (auto& worker : workers) {
    worker.join();
  }

  // Compute final result
  double finalResult = 0.0;
  for (double val : results) {
    finalResult += val;
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  std::cout << "Final Result: " << finalResult << std::endl;
  std::cout << "Execution Time: " << elapsed.count() << " seconds" << std::endl;

  return 0;
}
