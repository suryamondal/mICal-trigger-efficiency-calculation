/**
 * @file
 * @brief Implementation of module EventLoaderINO
 *
 * @copyright Copyright (c) 2023-2024 CERN
 * SPDX-License-Identifier: MIT
 */

#include "EventLoaderINO.h"
#include <cstdio>  // For sscanf

namespace corryvreckan {

  EventLoaderINO::EventLoaderINO(Configuration& config, std::shared_ptr<Detector> detector)
    : Module(config, detector), m_detector(detector), m_file(nullptr), m_tree(nullptr), m_currentEntry(0) {

    m_fileName = config.getPath("filename");
    m_treeName = config.get<std::string>("tree_name", "SNM");
    m_detectorRegion = config.getMatrix<int>("detectorRegion");
    m_eventLength = config.get<double>("event_length", Units::get<double>(1.0, "us"));
    m_timestampShift = config.get<double>("timestamp_shift", 0);
  }

  void EventLoaderINO::initialize() {
    // Open the ROOT file
    m_file = TFile::Open(m_fileName.c_str(), "READ");
    if(!m_file || m_file->IsZombie()) {
      LOG(ERROR) << "Failed to open ROOT file: " << m_fileName;
      throw std::runtime_error("Failed to open ROOT file");
    }

    // Get the tree
    m_tree = dynamic_cast<TTree*>(m_file->Get(m_treeName.c_str()));
    m_event = new SNM(m_tree);
    if(!m_tree) {
      LOG(ERROR) << "Failed to retrieve tree: " << m_treeName;
      throw std::runtime_error("Failed to retrieve TTree from ROOT file");
    }

    m_totalEntries = m_tree->GetEntries();
    LOG(DEBUG) << "Total entries in tree: " << m_totalEntries;

    // Initialize histograms
    hHitMap = new TH2F("hitMap", "Hit Map", 128, -0.5, 127.5, 128, -0.5, 127.5);
    hStripToT = new TH1F("stripToT", "Strip Charge", 100, 0, 500);
    hClipboardEventStart = new TH1D("clipboardEventStart", "Event Start Times", 3000, 0, 3000);
    hClipboardEventEnd = new TH1D("clipboardEventEnd", "Event End Times", 3000, 0, 3000);
    hClipboardEventDuration = new TH1D("clipboardEventDuration", "Event Durations", 3000, 0, 3000);
  }

  StatusCode EventLoaderINO::run(const std::shared_ptr<Clipboard>& clipboard) {
    PixelVector deviceData;

    bool dataLoaded = loadData(clipboard, deviceData);
    if(dataLoaded) {
      clipboard->putData(deviceData, m_detector->getName());
    }

    if(m_currentEntry >= m_totalEntries) {
      return StatusCode::EndRun;
    }

    return StatusCode::Success;
  }

  bool EventLoaderINO::loadData(const std::shared_ptr<Clipboard>& clipboard, PixelVector& deviceData_) {
    if(m_currentEntry >= m_totalEntries) {
      return false;
    }

    m_tree->GetEntry(m_currentEntry++);
    eventID = m_currentEntry;
    m_currentEntry++;
    detectorID = m_detector->getName();

    int l;
    if (std::sscanf(detectorID.c_str(), "RPC%d", &l) == 1) {
      std::vector<int> strips[2];
      for(int nj=0;nj<2;nj++)
        for(int kl=64-1; kl>=0; kl--)
          if((m_event->xydata[nj][l]>>kl)&0x01) {
            if (kl < m_detectorRegion[nj][0] || kl > m_detectorRegion[nj][1]) continue;
            int ntdc = kl % 8;
            int nTDCHits = m_event->xythit[nj][l][ntdc];
            time = nTDCHits ? m_event->xytime[nj][l][ntdc][0] * 0.1 : - 1000.0;
            double adjustedTime = time + m_timestampShift;
            if (adjustedTime < - 0.5 * m_eventLength ||
                adjustedTime >   0.5 * m_eventLength) continue;
            strips[nj].push_back(kl);
          }

      if (!int(strips[0].size()) || !int(strips[1].size())) {
        if(!clipboard->isEventDefined()) {
          double eventStart = - 0.5 * m_eventLength;
          double eventEnd =   + 0.5 * m_eventLength;
          clipboard->putEvent(std::make_shared<Event>(eventStart, eventEnd));
          clipboard->getEvent()->addTrigger(eventID, eventStart);
          hClipboardEventStart->Fill(Units::convert(eventStart, "ms"));
          hClipboardEventEnd->Fill(Units::convert(eventEnd, "ms"));
          hClipboardEventDuration->Fill(Units::convert(eventEnd - eventStart, "ms"));
        }
        return false;
      }

      for (auto xj : strips[0])
        for (auto yj : strips[1]) {
          LOG(DEBUG) << detectorID << " l=" << l
                     << " xj=" << xj << " yj=" << yj;
          double adjustedTime = 0;
          charge = 0;
          if(!clipboard->isEventDefined()) {
            double eventStart = adjustedTime - 0.5 * m_eventLength;
            double eventEnd = eventStart + 0.5 * m_eventLength;
            clipboard->putEvent(std::make_shared<Event>(eventStart, eventEnd));
            clipboard->getEvent()->addTrigger(eventID, eventStart);
            hClipboardEventStart->Fill(Units::convert(eventStart, "ms"));
            hClipboardEventEnd->Fill(Units::convert(eventEnd, "ms"));
            hClipboardEventDuration->Fill(Units::convert(eventEnd - eventStart, "ms"));
          }

          auto event = clipboard->getEvent();
          if(event->getTimestampPosition(adjustedTime) != Event::Position::DURING)
            return false;
          
          auto pixel = std::make_shared<Pixel>(detectorID, xj, yj,
                                               charge, charge, adjustedTime);
          deviceData_.push_back(pixel);

          hHitMap->Fill(stripID, 0);
          hStripToT->Fill(charge);

        }
    }
    return true;
  }

} // namespace corryvreckan
