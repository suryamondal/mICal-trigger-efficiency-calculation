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




const int ntrigLayers = 4;
const int trigLayers[ntrigLayers] = {0, 1, 2, 3};


const double expectedEventTime = -255;
const double triggerWindow     = 100;

const int        nside         =   2;
const int        nlayer        =  10;
const int        nstrip        =  64;
const int        nTDC          =   8;
const double     tdc_least     =   0.1;	 // in ns
const double     stripWidth     =   0.03; // in m

const double     maxtime       =  22.e3;      // in ns
const double     spdl_mpns      =   0.2;	      // m/ns
const double     cval_mpns     =   0.29979;   // light speed in m/ns
const double     cval_mps      =   0.29979e9; /* light speed in m/s */

const double     airDensity    = 0.0012; /* in g/cm3 */
const double     gapDensity    = 0.4;	 /* in g/cm3 */
const double     ironDensity   = 7.86;	 /* in g/cm3 */


const double   gapThickness      = 0.008; // 
const double   ironThickness     = 0.056; // 
const double   airGap            = 0.045; //
const double   rpcZShift         = 0; //
// const double   rpcXdistance      = 2.;	  // 
// const double   rpcYdistance      = 2.1;	  // 
// const double   rpcXOffset        = 0.;	  // 
// const double   rpcYOffset        = 0.;	  // 
// const double   moduleDistance    = 0.;	  //

const double positionUncertainty = stripWidth / TMath::Sqrt(12);


double getLayerZ(const int& layer) {
  return (airGap + ironThickness) * layer + rpcZShift;
};

int getILayer(const double& layer) {
  return std::round((layer - rpcZShift) / (airGap + ironThickness));
};


void LinearVectorFit(bool              isTime, // time iter
                     std::vector<TVector3>  pos,
                     std::vector<TVector2>  poserr,
                     std::vector<bool>      occulay,
                     TVector2         &slope,
                     TVector2         &inter,
                     TVector2         &chi2,
                     std::vector<TVector3> &ext,
                     std::vector<TVector3> &exterr) {

  double szxy[nside] = {0};
  double   sz[nside] = {0};
  double  sxy[nside] = {0};
  double   sn[nside] = {0};
  double  sz2[nside] = {0};

  double     slp[nside] = {-10000,-10000};
  double  tmpslp[nside] = {-10000,-10000};
  double intersect[nside] = {-10000,-10000};
  double    errcst[nside] = {-10000,-10000};
  double    errcov[nside] = {-10000,-10000};
  double    errlin[nside] = {-10000,-10000};

  for(int ij=0;ij<int(pos.size());ij++) {
    if(int(occulay.size()) && !occulay[ij]) {continue;}
    // cout << " ij " << ij << endl;
    double xyzval[3] = {pos[ij].X(), pos[ij].Y(), pos[ij].Z()};
    double xyerr[2]  = {poserr[ij].X(), poserr[ij].Y()};
    for(int nj=0;nj<nside;nj++) {
      szxy[nj] += xyzval[2]*xyzval[nj]/xyerr[nj];
      sz[nj]   += xyzval[2]/xyerr[nj];
      sz2[nj]  += xyzval[2]*xyzval[2]/xyerr[nj];
      sxy[nj]  += xyzval[nj]/xyerr[nj];
      sn[nj]   += 1/xyerr[nj];
    }   // for(int nj=0;nj<nside;nj++) {
  } // for(int ij=0;ij<int(pos.size());ij++){
  
  for(int nj=0;nj<nside;nj++) {
    if(sn[nj]>0. && sz2[nj]*sn[nj] - sz[nj]*sz[nj] !=0.) { 
      slp[nj] = (szxy[nj]*sn[nj] -
                 sz[nj]*sxy[nj])/(sz2[nj]*sn[nj] - sz[nj]*sz[nj]);
      tmpslp[nj] = slp[nj]; 
      if(isTime) { //time offset correction
        // if(fabs((cval*1.e-9)*slope+1)<3.30) { 
        tmpslp[nj] = -1./cval_mps;
        // }
      }
      intersect[nj] = sxy[nj]/sn[nj] - tmpslp[nj]*sz[nj]/sn[nj];

      double determ = (sn[nj]*sz2[nj] - sz[nj]*sz[nj]);
      errcst[nj] = sz2[nj]/determ;
      errcov[nj] = -sz[nj]/determ;
      errlin[nj] = sn[nj]/determ;
    }
  } // for(int nj=0;nj<nside;nj++) {
  slope.SetX(tmpslp[0]);
  slope.SetY(tmpslp[1]);
  inter.SetX(intersect[0]);
  inter.SetY(intersect[1]);

  // theta = atan(sqrt(pow(tmpslp[0],2.)+pow(tmpslp[1],2.)));
  // phi = atan2(tmpslp[1],tmpslp[0]);

  double sumx = 0, sumy = 0;
  ext.clear(); exterr.clear();
  for(int ij=0;ij<int(pos.size());ij++){
    TVector3 xxt;
    TVector3 xxtt;
    xxt.SetX(tmpslp[0]*pos[ij].Z()+intersect[0]);
    xxt.SetY(tmpslp[1]*pos[ij].Z()+intersect[1]);
    xxt.SetZ(pos[ij].Z());
    ext.push_back(xxt);
    xxtt.SetX(errcst[0] + 2*errcov[0]*pos[ij].Z()+
              errlin[0]*pos[ij].Z()*pos[ij].Z());
    xxtt.SetY(errcst[1] + 2*errcov[1]*pos[ij].Z()+
              errlin[1]*pos[ij].Z()*pos[ij].Z());
    exterr.push_back(xxtt);
    // cout << " " << int(exterr.size())
    // 	 << " " << 1./exterr.back().X()
    // 	 << " " << 1./exterr.back().Y() << endl;
    if(int(occulay.size())==0 || occulay[ij]) {
      sumx += pow(xxt.X()-pos[ij].X(), 2.)/poserr[ij].X(); 
      sumy += pow(xxt.Y()-pos[ij].Y(), 2.)/poserr[ij].Y(); 
    }
  } // for(int ij=0;ij<int(pos.size());ij++){
  chi2.SetX(sumx);
  chi2.SetY(sumy);
};


// Function to compute something
double computeFunction(std::vector<INO::PixelId> allPixels, INO::StripletId probeStripletId) {

  INO::INOCalibrationManager& inoCalibrationManager = INO::INOCalibrationManager::getInstance();

  std::vector<TVector3>  pos;
  std::vector<TVector2>  poserr;
  std::vector<bool>      occulay;
  TVector2         slope;
  TVector2         inter;
  TVector2         chi2;
  std::vector<TVector3> ext;
  std::vector<TVector3> exterr;
  pos.clear(); poserr.clear(); occulay.clear();
  for (auto pixel : allPixels) {
    if (probeStripletId.layer == pixel.layer) continue; // skip the probing layer
    INO::StripletId stripletId[2];
    TVector3 stripletPosition[2], stripletOrientation[2];
    TVector3 pixelPosition[2];
    for (int nj : {0, 1}) {
      stripletId[nj] = {
        pixel.module, pixel.row, pixel.column, pixel.layer,
        nj, pixel.strip[nj], int(pixel.strip[!nj] / 16)};
      inoCalibrationManager.getStripletPosition(stripletId[nj], stripletPosition[nj], stripletOrientation[nj]);
      pixelPosition[nj] = {
        nj ? 0 : (pixel.strip[nj] + 0.5) * stripWidth,
        nj ? (pixel.strip[nj] + 0.5) * stripWidth : 0, 0};
      pixelPosition[nj] += stripletPosition[nj];
      pixelPosition[nj].RotateX(stripletOrientation[nj].X() * TMath::DegToRad());
      pixelPosition[nj].RotateY(stripletOrientation[nj].Y() * TMath::DegToRad());
      pixelPosition[nj].RotateZ(stripletOrientation[nj].Z() * TMath::DegToRad());
    }
    TVector3 rawPos = {0, 0, getLayerZ(pixel.layer)};
    for (int nj : {0, 1})
      rawPos += pixelPosition[nj];
    pos.push_back(rawPos);
    poserr.push_back({positionUncertainty, positionUncertainty});
  }
  LinearVectorFit(0, pos, poserr, occulay, slope, inter, chi2, ext, exterr);

#ifdef isDebug
  for (int ij=0;ij<int(pos.size());ij++)
    std::cout << " X " << pos[ij].X() / stripWidth
              << " Y " << pos[ij].Y() / stripWidth
              << " extX " << ext[ij].X() / stripWidth
              << " extY " << ext[ij].Y() / stripWidth
              << " layer " << ext[ij].Z()
              << " layerI " << getILayer(ext[ij].Z())
              << std::endl;
#endif

  double stripResidual = std::numeric_limits<double>::quiet_NaN();
  for (auto extHit : ext) {
    int layer = getILayer(extHit.Z());
    if (probeStripletId.layer != layer) continue; // only the probing layer
    for (auto pixel : allPixels) {
      INO::StripletId stripletId[2];
      for (int nj : {0, 1})
        stripletId[nj] = {
          pixel.module, pixel.row, pixel.column, pixel.layer,
          nj, pixel.strip[nj], int(pixel.strip[!nj] / 16)};
      if (stripletId[0] != probeStripletId || stripletId[1] != probeStripletId) continue;

      TVector3 stripletPosition[2], stripletOrientation[2];
      TVector3 pixelPosition[2];
      for (int nj : {0, 1}) {
        inoCalibrationManager.getStripletPosition(stripletId[nj], stripletPosition[nj], stripletOrientation[nj]);
        pixelPosition[nj] = {
          nj ? 0 : (pixel.strip[nj] + 0.5) * stripWidth,
          nj ? (pixel.strip[nj] + 0.5) * stripWidth : 0, 0};
        pixelPosition[nj] += stripletPosition[nj];
        pixelPosition[nj].RotateX(stripletOrientation[nj].X() * TMath::DegToRad());
        pixelPosition[nj].RotateY(stripletOrientation[nj].Y() * TMath::DegToRad());
        pixelPosition[nj].RotateZ(stripletOrientation[nj].Z() * TMath::DegToRad());
      }
      TVector3 rawPos = {0, 0, getLayerZ(pixel.layer)};
      for (int nj : {0, 1})
        rawPos += pixelPosition[nj];
      for (int nj : {0, 1})
        if (stripletId[nj] == probeStripletId)
          stripResidual = extHit[nj] - rawPos[nj];
      if (!std::isnan(stripResidual)) break;
    }
    if (!std::isnan(stripResidual)) break;
  }

  return stripResidual;
}

// Thread worker function
void worker(std::queue<std::vector<INO::PixelId>>& taskQueue, const INO::StripletId& stripletId,
            std::mutex& queueMutex, std::condition_variable& cv,
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
    double result = computeFunction(task, stripletId);
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
  for (auto pixels : allPixelsInEvents)
    taskQueue.push(pixels);

  INO::StripletId stripletId = {0, 0, 0, 0, 0, 30, 1};

  // Create worker threads
  std::vector<std::thread> workers;
  for (unsigned int i = 0; i < maxworker; i++) {
    workers.emplace_back(worker, std::ref(taskQueue), std::ref(stripletId),
                         std::ref(queueMutex), std::ref(cv), 
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
