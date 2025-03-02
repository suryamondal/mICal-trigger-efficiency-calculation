#include "INOStorageManager.h"
#include <iostream>

using namespace INO;

// Singleton Instance
INOStorageManager& INOStorageManager::getInstance() {
  static INOStorageManager instance;
  return instance;
}

// Private Constructor
INOStorageManager::INOStorageManager() {}

// Get or create a ROOT file
TFile* INOStorageManager::getRootFile(const std::string& name, const std::string& mode) {
  auto it = rootFiles.find(name);
  if (it != rootFiles.end())
    return it->second.file;

  TFile* file = new TFile(name.c_str(), mode.c_str());
  if (!file || file->IsZombie()) {
    std::cerr << "Error opening file: " << name << std::endl;
    delete file;
    return nullptr;
  }

  rootFiles[name] = {file, {}, {}, {}};
  return file;
}

// Close a specific ROOT file and write all objects
void INOStorageManager::closeRootFile(const std::string& name) {
  auto it = rootFiles.find(name);
  if (it != rootFiles.end()) {
    TFile* file = it->second.file;
    if (file && file->IsOpen()) {
      file->cd();
      for (auto& item : it->second.trees) {
        auto dir = (TDirectory*)file->mkdir((item.first).c_str());
        dir->cd();
        for (auto& tree : item.second)
          tree->Write();
      }
      for (auto& item : it->second.histograms) {
        auto dir = (TDirectory*)file->mkdir((item.first).c_str());
        dir->cd();
        for (auto& hist : item.second)
          hist->Write();
      }
      for (auto item : it->second.graphs) {
        auto dir = (TDirectory*)file->mkdir((item.first).c_str());
        dir->cd();
        for (auto& graph : item.second)
          graph->Write();
      }
      file->Close();
    }
  }
}

// Add a tree to a specific file
void INOStorageManager::addTree(const std::string& filename,
                                const std::string& directory,
                                TTree* tree) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    tree->SetDirectory(0);
    it->second.trees[directory].push_back(tree);
  } else
    std::cerr << "Error: File " << filename << " not found!\n";
}

// Add a histogram to a specific file
void INOStorageManager::addHistogram(const std::string& filename,
                                     const std::string& directory,
                                     TH1* hist) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    hist->SetDirectory(0);
    it->second.histograms[directory].push_back(hist);
  } else
    std::cerr << "Error: File " << filename << " not found!\n";
}

// Add a graph to a specific file
void INOStorageManager::addGraph(const std::string& filename,
                                 const std::string& directory,
                                 TGraph* graph) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    // graph->SetDirectory(0);
    it->second.graphs[directory].push_back(graph);
  } else
    std::cerr << "Error: File " << filename << " not found!\n";
}

INOStorageManager::~INOStorageManager() {
  for (auto& entry : rootFiles) { 
    const std::string& name = entry.first;
    closeRootFile(name);
  }
  rootFiles.clear();
}
