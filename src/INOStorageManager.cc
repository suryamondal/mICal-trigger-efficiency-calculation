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
      for (auto* tree : it->second.trees)
        tree->Write();
      for (auto* hist : it->second.histograms)
        hist->Write();
      for (auto* graph : it->second.graphs)
        graph->Write();
      file->Close();
    }
  }
}

// Add a tree to a specific file
void INOStorageManager::addTree(const std::string& filename, TTree* tree) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    it->second.trees.push_back(tree);}
  else
    std::cerr << "Error: File " << filename << " not found!\n";
}

// Add a histogram to a specific file
void INOStorageManager::addHistogram(const std::string& filename, TH1* hist) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    it->second.histograms.push_back(hist);}
  else
    std::cerr << "Error: File " << filename << " not found!\n";
}

// Add a graph to a specific file
void INOStorageManager::addGraph(const std::string& filename, TGraph* graph) {
  auto it = rootFiles.find(filename);
  if (it != rootFiles.end()) {
    it->second.file->cd();
    it->second.graphs.push_back(graph);}
  else
    std::cerr << "Error: File " << filename << " not found!\n";
}

INOStorageManager::~INOStorageManager() {
  for (auto& entry : rootFiles) { 
    const std::string& name = entry.first;
    closeRootFile(name);
  }
  rootFiles.clear();
}
