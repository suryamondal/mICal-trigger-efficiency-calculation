/**
 * @file
 * @brief Definition of module EventLoaderINO
 *
 * @copyright Copyright (c) 2023-2024 CERN and the Corryvreckan authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

#ifndef EventLoaderINO_H
#define EventLoaderINO_H 1

#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <queue>
#include "core/module/Module.hpp"
#include "objects/Pixel.hpp"

#include "SNM.h"

namespace corryvreckan {

  class EventLoaderINO : public Module {
  public:
    // Constructors and destructors
    EventLoaderINO(Configuration& config, std::shared_ptr<Detector> detector);
    ~EventLoaderINO() {}

    // Standard algorithm functions
    void initialize() override;
    StatusCode run(const std::shared_ptr<Clipboard>& clipboard) override;

  private:
    struct Hit {
      int eventID;
      std::string detectorID;
      int stripID;
      double time;
      double charge;
    };

    using HitVector = std::vector<std::shared_ptr<Hit>>;

    std::string m_fileName;
    std::string m_treeName;
    std::shared_ptr<Detector> m_detector;
    Matrix<int> m_detectorRegion;
    double m_eventLength;
    double m_timestampShift;

    TFile* m_file;
    TTree* m_tree;
    SNM* m_event;
    Long64_t m_totalEntries;
    Long64_t m_currentEntry;

    // Tree branches
    int eventID;
    std::string detectorID;
    int stripID;
    double time;
    double charge;

    // Plots
    TH2F* hHitMap;
    TH1F* hStripToT;
    TH1D* hClipboardEventStart;
    TH1D* hClipboardEventEnd;
    TH1D* hClipboardEventDuration;

    // Additional helper function
    std::vector<Hit> readChunk();
    bool loadData(const std::shared_ptr<Clipboard>& clipboard, PixelVector&);
  };

} // namespace corryvreckan

#endif // EventLoaderINO_H
