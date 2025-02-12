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

TTimeStamp MAX_TIME(2060, 1, 1, 0, 0, 0, 0, true);

struct FileInfo {
  std::string filename;
  TTimeStamp timestamp;
};

std::vector<FileInfo> getFileList(const std::string &path) {
  std::vector<FileInfo> files;
  void *dir = gSystem->OpenDirectory(path.c_str());
  const char *entry;
  while ((entry = gSystem->GetDirEntry(dir))) {
    std::string name(entry);
    if (name.find("SNM_RPCv4t_evtraw") != std::string::npos && name.find(".root") != std::string::npos) {
      std::cout << name << std::endl;
      std::string time_str = name.substr(18, 15);
      std::cout << time_str << std::endl;
      int year = stoi(time_str.substr(0, 4));
      int month = stoi(time_str.substr(4, 2));
      int day = stoi(time_str.substr(6, 2));
      int hour = stoi(time_str.substr(9, 2));
      int minute = stoi(time_str.substr(11, 2));
      int second = stoi(time_str.substr(13, 2));
      TTimeStamp timestamp(year, month, day, hour, minute, second, 0, true);
      files.push_back({name, timestamp});
    }
  }
  return files;
}

std::vector<std::string> getHistogramNames(TFile *file) {
  std::vector<std::string> names;
  TDirectory *dir = (TDirectory *)file->Get("ConstantStripTimeDelay");
  if (!dir) return names;
  TIter next(dir->GetListOfKeys());
  TKey *key;
  while ((key = (TKey *)next()))
    if (std::string(key->GetClassName()) == "TH1D")
      names.push_back(key->GetName());
  return names;
}

void processFiles(const std::string &directory) {
  std::vector<FileInfo> files = getFileList(directory);
  std::sort(files.begin(), files.end(), [](const FileInfo &a, const FileInfo &b) { return a.timestamp < b.timestamp; });
  std::map<std::string, TH1D*> mergedHists;
  std::map<std::string, double> means;
  TTimeStamp startTime;
  for (size_t i = 0; i < int(files.size()); i++) {
    TFile file((directory + "/" + files[i].filename).c_str(), "READ");
    if (!file.IsOpen()) continue;
    std::vector<std::string> histNames = getHistogramNames(&file);
    for (const auto &histName : histNames) {
      TH1D *hist = (TH1D*)file.Get(("ConstantStripTimeDelay/" + histName).c_str());
      if (!hist) continue;
      double mean = hist->GetMean();
      auto it = mergedHists.find(histName);
      if (it == mergedHists.end()) {
        mergedHists[histName] = hist;
        mergedHists[histName]->SetDirectory(0);
        means[histName] = mean;
        startTime = files[i].timestamp;
      } else {
        if (std::fabs(mean - means[histName]) > 0.5 ||
            i == int(files.size()) - 1) { 
          TF1 gauss("gaus", "gaus",
                    means[histName] - 10,
                    means[histName] + 10);
          mergedHists[histName]->Fit(&gauss, "RQ");
          double center = gauss.GetParameter(1);
          int m, r, c, l, s; char axis;
          std::sscanf(histName.c_str(), "m%d_r%d_c%d_l%d_%c_s%d", &m, &r, &c, &l, &axis, &s);
          std::cout << "m: " << m << ", r: " << r << ", c: " << c
                    << ", l: " << l << ", axis: " << axis << ", s: " << s << std::endl;
          double starttime = startTime.AsDouble();
          double endtime = (i == int(files.size()) - 1) ? MAX_TIME.AsDouble() : files[i + 1].timestamp.AsDouble();
          std::cout << starttime << " " << endtime << " " << center << std::endl;
          mergedHists.clear();
          means.clear();
          startTime = files[i].timestamp;
        } else {
          mergedHists[histName]->Add(hist);
          means[histName] = mergedHists[histName]->GetMean();
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <directory>" << std::endl;
    return 1;
  }
  processFiles(argv[1]);
  return 0;
}
