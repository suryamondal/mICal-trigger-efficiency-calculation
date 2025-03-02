#pragma once

#include <memory>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>

namespace INO {

  class INOStorageManager {
  public:
    static INOStorageManager& getInstance();

    TFile* getRootFile(const std::string& name, const std::string& mode);
    void closeRootFile(const std::string& name);
    
    // Methods to associate objects with ROOT files
    void addTree(const std::string& filename,
                 const std::string& directory, TTree* tree);
    void addHistogram(const std::string& filename,
                 const std::string& directory, TH1* hist);
    void addGraph(const std::string& filename,
                 const std::string& directory, TGraph* graph);

    ~INOStorageManager();

  private:
    INOStorageManager();
    INOStorageManager(const INOStorageManager&) = delete;
    INOStorageManager& operator=(const INOStorageManager&) = delete;

    struct RootFileData {
      TFile* file;
      std::map<std::string, std::vector<TTree*>> trees;
      std::map<std::string, std::vector<TH1*>> histograms;
      std::map<std::string, std::vector<TGraph*>> graphs;
    };

    std::unordered_map<std::string, RootFileData> rootFiles;
  };

} // namespace INO
