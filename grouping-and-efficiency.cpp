

// #define isDebug

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <bitset>
#include <memory>
#include <csignal>
#include <algorithm>

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

using namespace std;


const int ntrigLayers = 4;
const int trigLayers[ntrigLayers] = {0, 1, 2, 3};


const double expectedEventTime = -255;
const double triggerWindow     = 100;

const int        nside         =   2;
const int        nlayer        =  10;
const int        nstrip        =  64;
const int        nTDC          =   8;
const double     tdc_least     =   0.1;	 // in ns
const double     stripwidth     =   0.03; // in m

const double     maxtime       =  22.e3;      // in ns
const double     spdl_mps      =   0.2;	      // m/ns
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


double getLayerZ(const int& layer) {
  return (airGap + ironThickness) * layer + rpcZShift;
};

int getILayer(const double& layer) {
  return (layer - rpcZShift) / (airGap + ironThickness);
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
    double xyzval[3] = {pos[ij].X(),
                        pos[ij].Y(),
                        pos[ij].Z()};
    double xyerr[2]  = {poserr[ij].X(),
                        poserr[ij].Y()};
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
  TVector3 xxt;
  TVector3 xxtt;
  for(int ij=0;ij<int(pos.size());ij++){
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

  // INO::INOStorageManager& inoStorageManager = INO::INOStorageManager::getInstance();


  char datafile[1000] = {};
  strncpy(datafile,argv[1],1000);
  char outfile[1000] = {};
  strncpy(outfile,argv[2],1000);
  Long64_t nentrymn = stoi(argv[3]);
  Long64_t nentrymx = stoi(argv[4]);

  TFile* fileOut = new TFile((std::string(outfile) + ".root").c_str(), "recreate");
  // fileOut->cd();

  // auto fileOut = inoStorageManager.getRootFile(std::string(outfile) + ".root", "recreate");
  // if(!fileOut) return 0;

  std::map<INO::StripId, TH1D*> stripTimeDelay;
  std::map<INO::SideId, TH1D*> positionResidual;
  std::map<INO::SideId, TH2D*> layerEfficiency;
  std::map<std::string, TH1D*> eventMetaHistograms;

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
    inoEvent->setEventTime(eventTime.AsDouble() + (5 * 3600) + (30 * 60));
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

    std::shared_ptr<INO::INOTimeGroupingModule> inoTimeGrouping = std::make_shared<INO::INOTimeGroupingModule>(inoEvent);
    inoTimeGrouping->process();

#ifdef isDebug
    for (const auto* hit : inoEvent->getHits()) {
      INO::StripId stripId = hit->stripId;
      double calibratedTime = inoEvent->getCalibratedLeadingTimes(stripId)[0];
      double low = inoEvent->getLowestCalibratedLeadingTime();
      double high = inoEvent->getHighestCalibratedLeadingTime();
      std::cout << std::setw(10) << "Module: " << stripId.module
                << " | Row: " << stripId.row
                << " | Column: " << stripId.column
                << " | Layer: " << stripId.layer
                << " | Side: " << (stripId.side ? "Y" : "X")
                << " | Strip: " << stripId.strip
                << " | Groups: " << inoEvent->getTimeGroupId(stripId).size()
                << " | Calibrated Time: "
                << std::fixed << std::setprecision(3) << calibratedTime << " ns"
        // << " | Low Time: " << low << " ns"
        // << " | High Time: " << high << " ns"
                << std::endl;
    }
#endif

    // compute event time
    double triggerTime = std::numeric_limits<double>::quiet_NaN();
    for (const auto* hit : inoEvent->getHits()) {
      INO::StripId stripId = hit->stripId;
      auto groupIds = inoEvent->getTimeGroupId(stripId);
      if (!int(groupIds.size())) continue;
      if (groupIds[0] == 0)
        triggerTime = std::get<1>(inoEvent->getTimeGroupInfo(stripId)[0]);
    }
    auto triggerTimeHist = eventMetaHistograms.find("triggerTime");
    if (triggerTimeHist == eventMetaHistograms.end()) {
      eventMetaHistograms["triggerTime"] = new TH1D("triggerTime",
                                                    "triggerTime",
                                                    200, -25, 25);
      eventMetaHistograms["triggerTime"]->SetDirectory(0);
    }
    eventMetaHistograms["triggerTime"]->Fill(triggerTime);


    std::map<INO::LayerId, int> stripHits[2];
    for (const auto* hit : inoEvent->getHits()) {
      INO::StripId stripId = hit->stripId;
      if(!int(inoEvent->getCalibratedLeadingTimes(stripId).size())) continue;
      // if (std::fabs(inoEvent->getCalibratedLeadingTimes(stripId)[0] - expectedEventTime) > triggerWindow * 0.5) continue;
      auto groupIds = inoEvent->getTimeGroupId(stripId);
      if (std::find(groupIds.begin(), groupIds.end(), 0) == groupIds.end()) continue; // only group 0
      stripHits[hit->stripId.side][{hit->stripId.module,
            hit->stripId.row,
            hit->stripId.column,
            hit->stripId.layer}] = hit->stripId.strip;
    }
    std::vector<INO::PixelId> allPixels;
    for (auto xStripHit : stripHits[0])
      for (auto yStripHit : stripHits[1])
        if (xStripHit.first == yStripHit.first)
          allPixels.push_back({xStripHit.first.module,
                               xStripHit.first.row,
                               xStripHit.first.column,
                               xStripHit.first.layer,
                               {xStripHit.second, yStripHit.second} });

    std::vector<TVector3>  pos;
    std::vector<TVector2>  poserr;
    std::vector<bool>      occulay;
    TVector2         slope;
    TVector2         inter;
    TVector2         chi2;
    std::vector<TVector3> ext;
    std::vector<TVector3> exterr;
    for (auto pixel : allPixels) {
      TVector3 rawPos = {(pixel.strip[0] + 0.5) * stripwidth,
                         (pixel.strip[1] + 0.5) * stripwidth,
                         getLayerZ(pixel.layer)};
      TVector3 rpcPosition, rpcOrientation;
      inoCalibrationManager
        .getLayerPosition({pixel.module, pixel.row, pixel.column,
                           pixel.layer},
          pixel.strip[0] < 32 ? 0 : 1,
          pixel.strip[1] < 32 ? 0 : 1,
          rpcPosition, rpcOrientation);
      rpcPosition.SetZ(0);
      rawPos += rpcPosition;
      rawPos.RotateX(-rpcOrientation.X() * TMath::DegToRad());
      rawPos.RotateY(-rpcOrientation.Y() * TMath::DegToRad());
      rawPos.RotateZ(-rpcOrientation.Z() * TMath::DegToRad());
      pos.push_back(rawPos);
      poserr.push_back({0.008, 0.008});
    }
    LinearVectorFit(0, pos, poserr, occulay, slope, inter, chi2, ext, exterr);

    for (auto extHit : ext) {
      int layer = getILayer(extHit.Z());
      for (auto pixel : allPixels) {
        if (layer != pixel.layer) continue;
        TVector3 rawPos = {(pixel.strip[0] + 0.5) * stripwidth,
                           (pixel.strip[1] + 0.5) * stripwidth,
                           getLayerZ(pixel.layer)};
        TVector3 rpcPosition, rpcOrientation;
        inoCalibrationManager
          .getLayerPosition({pixel.module, pixel.row, pixel.column,
                             pixel.layer},
            pixel.strip[0] < 32 ? 0 : 1,
            pixel.strip[1] < 32 ? 0 : 1,
            rpcPosition, rpcOrientation);
        rpcPosition.SetZ(0);
        rawPos += rpcPosition;
        rawPos.RotateX(-rpcOrientation.X() * TMath::DegToRad());
        rawPos.RotateY(-rpcOrientation.Y() * TMath::DegToRad());
        rawPos.RotateZ(-rpcOrientation.Z() * TMath::DegToRad());
        for (int nj : {0, 1}) {
          // position
          INO::SideId sideId = {pixel.module, pixel.row, pixel.column, layer, nj};
          auto it = positionResidual.find(sideId);
          if (it == positionResidual.end()) {
            std::string sideName = INO::getSideName(sideId);
            positionResidual[sideId] = new TH1D(sideName.c_str(),
                                                sideName.c_str(),
                                                500, -0.25, 0.25);
            positionResidual[sideId]->SetDirectory(0);
          }
          positionResidual[sideId]->Fill(extHit[nj] - rawPos[nj]);
          // time
          INO::StripId stripId = {pixel.module, pixel.row, pixel.column,
                                  layer, nj, pixel.strip[nj]};
          auto itt = stripTimeDelay.find(stripId);
          if (itt == stripTimeDelay.end()) {
            std::string stripName = INO::getStripName(stripId);
            stripTimeDelay[stripId] = new TH1D(stripName.c_str(),
                                               stripName.c_str(),
                                               200, -50, 50);
            stripTimeDelay[stripId]->SetDirectory(0);
          }
          if(int(inoEvent->getCalibratedLeadingTimes(stripId).size())) {
            double time = inoEvent->getCalibratedLeadingTimes(stripId)[0];
            time -= extHit[!nj] / spdl_mps;
            stripTimeDelay[stripId]->Fill(time);
          }
        }
      }
#ifdef isDebug
      std::cout << " extX " << extHit.X() / stripwidth
                << " extY " << extHit.Y() / stripwidth
                << " extZ " << layer
                << std::endl; 
#endif
    }

    if (stopFlag) {
      std::cout << "Exiting loop due to Ctrl+C.\n";
      break;
    }

  } // for(Long64_t iev=nentrymn;iev<nentry;iev++) {
  fileIn->Close();

  TDirectory* dir = fileOut->mkdir("EventMeta");
  dir->cd();
  for (auto& item : eventMetaHistograms)
    if(item.second)
      item.second->Write();
  dir = fileOut->mkdir("PositionResidual");
  dir->cd();
  for (auto& item : positionResidual)
    if(item.second)
      item.second->Write();
  dir = fileOut->mkdir("StripTimeDelay");
  dir->cd();
  for (auto& item : stripTimeDelay)
    if(item.second)
      item.second->Write();
  fileOut->Close();

  for (auto& item : stripTimeDelay)
    if(item.second)
      delete item.second;
  for (auto& item : positionResidual)
    if(item.second)
      delete item.second;
  for (auto& item : eventMetaHistograms)
    if(item.second)
      delete item.second;

  return 0;
}; // main

