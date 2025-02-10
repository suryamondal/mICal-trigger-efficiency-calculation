

// #define isDebug

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <bitset>
#include <memory>
#include <csignal>

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
#include "DynamicHistogram.h"

using namespace std;


const int ntrigLayers = 4;
const int trigLayers[ntrigLayers] = {0, 1, 2, 3};


const int        nside         =   2;
const int        nlayer        =  10;
const int        nstrip        =  64;
const int        nTDC          =   8;
const double     tdc_least     =   0.1;	 // in ns
const double     strpwidth     =   0.03; // in m

const double     maxtime       =  22.e3;      // in ns
const double     spdl          =   5.;	      // ns/m
const double     cval_mpns     =   0.29979;   // light speed in m/ns
const double     cval_mps      =   0.29979e9; /* light speed in m/s */

const double     airDensity    = 0.0012; /* in g/cm3 */
const double     gapDensity    = 0.4;	 /* in g/cm3 */
const double     ironDensity   = 7.86;	 /* in g/cm3 */


const double   gapThickness      = 0.008; // 
const double   ironThickness     = 0.056; // 
const double   airGap            = 0.045; //
const double   rpcZShift         =-0.005; // 
const double   rpcXdistance      = 2.;	  // 
const double   rpcYdistance      = 2.1;	  // 
const double   rpcXOffset        = 0.;	  // 
const double   rpcYOffset        = 0.;	  // 
const double   moduleDistance    = 0.;	  //


// void LinearVectorFit(bool              isTime, // time iter
// 		     vector<TVector3>  pos,
// 		     vector<TVector2>  poserr,
// 		     vector<bool>      occulay,
// 		     TVector2         &slope,
// 		     TVector2         &inter,
// 		     TVector2         &chi2,
// 		     vector<TVector3> &ext,
// 		     vector<TVector3> &exterr) {
  
//   double szxy[nside] = {0};
//   double   sz[nside] = {0};
//   double  sxy[nside] = {0};
//   double   sn[nside] = {0};
//   double  sz2[nside] = {0};
  
//   double     slp[nside] = {-10000,-10000};
//   double  tmpslp[nside] = {-10000,-10000};
//   double intersect[nside] = {-10000,-10000};
//   double    errcst[nside] = {-10000,-10000};
//   double    errcov[nside] = {-10000,-10000};
//   double    errlin[nside] = {-10000,-10000};
  
//   for(int ij=0;ij<int(pos.size());ij++) {
//     if(int(occulay.size()) && !occulay[ij]) {continue;}
//     // cout << " ij " << ij << endl;
//     double xyzval[3] = {pos[ij].X(),
// 			pos[ij].Y(),
// 			pos[ij].Z()};
//     double xyerr[2]  = {poserr[ij].X(),
// 			poserr[ij].Y()};
//     for(int nj=0;nj<nside;nj++) {
//       szxy[nj] += xyzval[2]*xyzval[nj]/xyerr[nj];
//       sz[nj]   += xyzval[2]/xyerr[nj];
//       sz2[nj]  += xyzval[2]*xyzval[2]/xyerr[nj];
//       sxy[nj]  += xyzval[nj]/xyerr[nj];
//       sn[nj]   += 1/xyerr[nj];
//     }   // for(int nj=0;nj<nside;nj++) {
//   } // for(int ij=0;ij<int(pos.size());ij++){
  
//   for(int nj=0;nj<nside;nj++) {
//     if(sn[nj]>0. && sz2[nj]*sn[nj] - sz[nj]*sz[nj] !=0.) { 
//       slp[nj] = (szxy[nj]*sn[nj] -
// 		 sz[nj]*sxy[nj])/(sz2[nj]*sn[nj] - sz[nj]*sz[nj]);
//       tmpslp[nj] = slp[nj]; 
//       if(isTime) { //time offset correction
//         // if(fabs((cval*1.e-9)*slope+1)<3.30) { 
// 	tmpslp[nj] = -1./cval;
// 	// }
//       }
//       intersect[nj] = sxy[nj]/sn[nj] - tmpslp[nj]*sz[nj]/sn[nj];

//       double determ = (sn[nj]*sz2[nj] - sz[nj]*sz[nj]);
//       errcst[nj] = sz2[nj]/determ;
//       errcov[nj] = -sz[nj]/determ;
//       errlin[nj] = sn[nj]/determ;
//     }
//   } // for(int nj=0;nj<nside;nj++) {
//   slope.SetX(tmpslp[0]);
//   slope.SetY(tmpslp[1]);
//   inter.SetX(intersect[0]);
//   inter.SetY(intersect[1]);
  
//   // theta = atan(sqrt(pow(tmpslp[0],2.)+pow(tmpslp[1],2.)));
//   // phi = atan2(tmpslp[1],tmpslp[0]);
  
//   double sumx = 0, sumy = 0;
//   ext.clear(); exterr.clear();
//   TVector3 xxt;
//   TVector3 xxtt;
//   for(int ij=0;ij<int(pos.size());ij++){
//     xxt.SetX(tmpslp[0]*pos[ij].Z()+intersect[0]);
//     xxt.SetY(tmpslp[1]*pos[ij].Z()+intersect[1]);
//     xxt.SetZ(pos[ij].Z());
//     ext.push_back(xxt);
//     xxtt.SetX(errcst[0] + 2*errcov[0]*pos[ij].Z()+
// 	      errlin[0]*pos[ij].Z()*pos[ij].Z());
//     xxtt.SetY(errcst[1] + 2*errcov[1]*pos[ij].Z()+
// 	      errlin[1]*pos[ij].Z()*pos[ij].Z());
//     exterr.push_back(xxtt);
//     // cout << " " << int(exterr.size())
//     // 	 << " " << 1./exterr.back().X()
//     // 	 << " " << 1./exterr.back().Y() << endl;
//     if(int(occulay.size())==0 || occulay[ij]) {
//       sumx += pow(xxt.X()-pos[ij].X(), 2.)/poserr[ij].X(); 
//       sumy += pow(xxt.Y()-pos[ij].Y(), 2.)/poserr[ij].Y(); 
//     }
//   } // for(int ij=0;ij<int(pos.size());ij++){
//   chi2.SetX(sumx);
//   chi2.SetY(sumy);
// };


volatile sig_atomic_t stopFlag = 0; // Global flag to detect Ctrl+C
// Signal handler function
void signalHandler(int signum) {
  std::cout << "\nInterrupt signal (" << signum << ") received. Stopping loop...\n";
  stopFlag = 1;  // Set flag to break loop
}

int main(int argc, char** argv) {

  /* 
     argv[0]  : main
     argv[1]  : inputfilename
     argv[2]  : outputfilename
     argv[3]  : start event
     argv[4]  : end event
     argv[5]  : output file number
     argv[6]  : itermodule
     argv[7]  : iterxrow
     argv[8]  : iteryrow
     argv[9]  : iterlayer
     argv[10] : (empty or additional argument)
  */
  
  // #ifdef isIter
  //   int itermodule = stoi(argv[5]);
  //   int iterxrow = stoi(argv[6]);
  //   int iteryrow = stoi(argv[7]);
  //   int iterlayer = stoi(argv[8]);
  // #endif	// #ifdef isIter

  // Register signal handler for Ctrl+C
  signal(SIGINT, signalHandler);

  const char *sideMark[nside] = {"x","y"};

  Double_t evetimeFill, evesepFill, recotimeFill, recosepFill;
  
  unsigned int triggerinfo_ref = 0;
  for(int ij=0;ij<ntrigLayers;ij++) {
    triggerinfo_ref<<=1;
    triggerinfo_ref+=1;}


  /*
    C : a character string terminated by the 0 character
    B : an 8 bit signed integer (Char_t)
    b : an 8 bit unsigned integer (UChar_t)
    S : a 16 bit signed integer (Short_t)
    s : a 16 bit unsigned integer (UShort_t)
    I : a 32 bit signed integer (Int_t)
    i : a 32 bit unsigned integer (UInt_t)
    F : a 32 bit floating point (Float_t)
    f : a 24 bit floating point with truncated mantissa (Float16_t)
    D : a 64 bit floating point (Double_t)
    d : a 24 bit truncated floating point (Double32_t)
    L : a 64 bit signed integer (Long64_t)
    l : a 64 bit unsigned integer (ULong64_t)
    G : a long signed integer, stored as 64 bit (Long_t)
    g : a long unsigned integer, stored as 64 bit (ULong_t)
    O : [the letter o, not a zero] a boolean (Bool_t)
  */

  INO::INOCalibrationManager& inoCalibrationManager = INO::INOCalibrationManager::getInstance();
  inoCalibrationManager.loadTimeCalibration("calibration-data/SNM_RPCv4t_evtraw_20181212_193056_RawTimeOffset.txt");


  char datafile[300] = {};
  strncpy(datafile,argv[1],300);
  char outfile[300] = {};
  strncpy(outfile,argv[2],300);
  Long64_t nentrymn = stoi(argv[3]);
  Long64_t nentrymx = stoi(argv[4]);

  // TFile* fileOut = new TFile(outfile, "recreate");
  // fileOut->cd();
  std::map<INO::StripId, DynamicHistogram> constantStripTimeDelay;

  TFile* fileIn = new TFile(datafile, "read");

  if(fileIn->IsZombie()) return 0;

  TTree *event_tree = (TTree*)fileIn->Get("SNM");
  SNM *event = new SNM(event_tree);
  event->Loop();
  
  Long64_t start_s = clock();

  Long64_t nentry = event_tree->GetEntries();
  for(Long64_t iev=nentrymn; iev<=TMath::Min(nentry - 1, nentrymx); iev++) {
      
    if(iev%1000==0) {
      Long64_t stop_s = clock();
      cout << " iev " << iev
           << " time " << (stop_s-start_s)/Double_t(CLOCKS_PER_SEC)
           << endl;
    }
  
    fileIn->cd();
    event_tree->GetEntry(iev);

    std::shared_ptr<INO::INOEvent> inoEvent = std::make_shared<INO::INOEvent>();

    TTimeStamp eventTime = event->evetime[0];
    inoEvent->setEventTime(eventTime.AsDouble());
    evesepFill = inoEvent->getEventTime() - evetimeFill;
    evetimeFill = inoEvent->getEventTime();

// #ifdef isDebug
//     cout << " time " << eventTime << endl;
// #endif  // #ifdef isDebug

    // setting rawTDCs
    for(int ij=0;ij<nlayer;ij++)
      for(int nj=0;nj<nside;nj++)
        for (int ntdc = 0; ntdc < 8; ntdc++) {
          int nTDCHits = event->xythit[nj][ij][ntdc];
          for (int tc = 0; tc < nTDCHits; tc++) {
            int rawTDCl = event->xytime[nj][ij][ntdc][tc] * tdc_least;
            int rawTDCt = rawTDCl + event->plWidth[nj][ij][ntdc][tc] * tdc_least;
            inoEvent->addTDC(INO::TDCId{0,0,0,ij,nj,ntdc}, rawTDCl, 0);
            inoEvent->addTDC(INO::TDCId{0,0,0,ij,nj,ntdc}, rawTDCt, 1);
          }
        }
    // setting strip hits
    for(int ij=0;ij<nlayer;ij++)
      for(int nj=0;nj<nside;nj++)
        for(int kl=nstrip-1; kl>=0; kl--)
          if((event->xydata[nj][ij]>>kl)&0x01)
            inoEvent->addHit(INO::StripId{0,0,0,ij,nj,kl});

    for (const auto* hit : inoEvent->getHits()) {
      INO::StripId stripId = hit->stripId;
      for (auto time : inoEvent->getRawLeadingTimes(stripId))
        constantStripTimeDelay[stripId].fillValue(time);
    }

    std::shared_ptr<INO::INOTimeGroupingModule> inoTimeGrouping = std::make_shared<INO::INOTimeGroupingModule>(inoEvent);
    // inoTimeGrouping->process();

    // #ifdef isDebug
    //     for (const auto* hit : inoEvent->getHits()) {
    //       INO::StripId stripId = hit->stripId;
    //       double rawTime = inoEvent->getRawLeadingTime(stripId);
    //       double low = inoEvent->getLowestCalibratedLeadingTime();
    //       double high = inoEvent->getHighestCalibratedLeadingTime();
    //       std::cout << std::setw(10) << "Module: " << stripId.module
    //                 << " | Row: " << stripId.row
    //                 << " | Column: " << stripId.column
    //                 << " | Layer: " << stripId.layer
    //                 << " | Side: " << (stripId.side ? "Y" : "X")
    //                 << " | Strip: " << stripId.strip
    //                 << " | Groups: " << inoEvent->getTimeGroupId(stripId).size()
    //                 << " | Raw Time: "
    //                 << std::fixed << std::setprecision(3) << rawTime << " ns"
    //                 << " | Low Time: " << low << " ns"
    //                 << " | High Time: " << high << " ns"
    //                 << std::endl;
    //     }
    // #endif

    if (stopFlag) {
      std::cout << "Exiting loop due to Ctrl+C.\n";
      break;
    }

  } // for(Long64_t iev=nentrymn;iev<nentry;iev++) {

  for (auto item : constantStripTimeDelay) {
    auto stripId = item.first;
    auto fistBin = item.second.getPeak();
    if (fistBin.count < 5) continue;
    inoCalibrationManager.setTimeCalibration(stripId, fistBin.center);
#ifdef isDebug
    std::cout << std::setw(10) << "Module: " << stripId.module
              << " | Row: " << stripId.row
              << " | Column: " << stripId.column
              << " | Layer: " << stripId.layer
              << " | Side: " << (stripId.side ? "Y" : "X")
              << " | Strip: " << stripId.strip
              << " | Count: " << fistBin.count
              << " | Center: " << fistBin.center
              << std::endl;
#endif
  }

  inoCalibrationManager.writeTimeCalibration(std::string(outfile) + "_RawTimeOffset.txt");
  fileIn->Close();

  return 0;
}; // main

