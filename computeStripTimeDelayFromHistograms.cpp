#include <iostream>
#include <vector>
#include <string>
#include <TFile.h>
#include <TDirectory.h>
#include <TH1D.h>
#include <TKey.h>
#include <TF1.h>
#include <TSystem.h>
#include <TTimeStamp.h>
#include <map>

#include "INOCalibrationManager.h"


std::vector<std::string> getHistogramNames(TFile *file) {
  std::vector<std::string> names;
  TDirectory *dir = (TDirectory *)file->Get("StripTimeDelay");
  if (!dir) return names;
  TIter next(dir->GetListOfKeys());
  TKey *key;
  while ((key = (TKey *)next()))
    if (std::string(key->GetClassName()) == "TH1D")
      names.push_back(key->GetName());
  return names;
}

void processFiles(const std::string &filename) {
  std::cout << filename << std::endl;
  INO::INOCalibrationManager& instance = INO::INOCalibrationManager::getInstance();
  TFile file(filename.c_str(), "READ");
  if (!file.IsOpen()) return;
  std::cout << filename << std::endl;
  std::vector<std::string> histNames = getHistogramNames(&file);
  for (const auto &histName : histNames) {
    std::cout << histName << std::endl;
    TH1D *hist = (TH1D*)file.Get(("StripTimeDelay/" + histName).c_str());
    if (!hist) continue;
    if (hist->GetEntries() < 100) continue;
    double mean = hist->GetBinCenter(hist->GetMaximumBin());
    TF1 gauss("gaus", "gaus", mean - 6, mean + 6);
    hist->Fit(&gauss, "RQ");
    double center = gauss.GetParameter(1);
    int m, r, c, l, s; char axis;
    std::sscanf(histName.c_str(), "m%d_r%d_c%d_l%d_%c_s%d", &m, &r, &c, &l, &axis, &s);
    std::cout << "m: " << m << ", r: " << r << ", c: " << c
              << ", l: " << l << ", axis: " << axis << ", s: " << s
              << ", center " << center << std::endl;
    instance.setStripTimeDelay({m, r, c, l, axis == 'x' ? 0 : 1, s}, center);
  }
}


int main(int argc, char *argv[]) {

  processFiles(argv[1]);
  
  return 0;
}
