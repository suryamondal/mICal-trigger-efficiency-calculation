
#include "TimeGroupingModule.h"

// root
#include <TString.h>

using namespace INO;


TimeGroupingModule::TimeGroupingModule(std::shared_ptr<INOEvent> data) :
  m_inoEvent(data)
{
  // Fill time Histogram:
  m_usedPars.tRange[0] = data->getLowestCalibratedLeadingTime();
  m_usedPars.tRange[1] = data->getHighestCalibratedLeadingTime();
  m_usedPars.rebinningFactor = 1;
  m_usedPars.fillSigmaN = 3.0;
  // Search peaks:
  m_usedPars.limitSigma[0] = 1.0;
  m_usedPars.limitSigma[1] = 15.0;
  m_usedPars.fitRangeHalfWidth = 5.0;
  m_usedPars.removeSigmaN = 7.;
  m_usedPars.fracThreshold = 0.05;
  m_usedPars.maxGroups = 20;
  // Sort groups:
  m_usedPars.expectedSignalTime[1] =  -500.0;
  m_usedPars.expectedSignalTime[0] = 25000.0;
  m_usedPars.expectedSignalTime[2] =  -250.0;
  m_usedPars.signalLifetime = 25000.0;
  // Signal group selection:
  m_usedPars.acceptSigmaN = 7.0;
  m_usedPars.writeGroupInfo = true;
  // Handle out-of-range clusters:
  m_usedPars.includeOutOfRangeClusters = true;
  m_usedPars.clsSigma = 2.0;
}


void TimeGroupingModule::process()
{
  if (int(m_inoEvent->getEntries()) < 4) return;

  // declare and fill the histogram shaping each cluster with a normalised gaussian
  // G(cluster time, resolution)
  TH1D h_clsTime;
  createAndFillHistorgram(h_clsTime);

  // now we search for peaks and when we find one we remove it from the distribution, one by one.
  std::vector<GroupInfo> groupInfoVector; // Gauss parameters (integral, center, sigma)

  // performing the search
  searchGausPeaksInHistogram(h_clsTime, groupInfoVector);
  // resize to max
  resizeToMaxSize(groupInfoVector);
  // sorting background groups
  sortBackgroundGroups(groupInfoVector);
  // sorting signal groups
  sortSignalGroups(groupInfoVector);

  // assign the groupID to clusters
  assignGroupIdsToClusters(h_clsTime, groupInfoVector);

} // end of event


void TimeGroupingModule::createAndFillHistorgram(TH1D& hist)
{

  // minimise the range of the histogram removing empty bins at the edge
  // to speed up the execution time.

  int totClusters = m_inoEvent->getEntries();

  double tRangeHigh = m_usedPars.tRange[1];
  double tRangeLow  = m_usedPars.tRange[0];

  int nBin = tRangeHigh - tRangeLow;
  if (nBin < 1) nBin = 1;
  nBin *= m_usedPars.rebinningFactor;
  if (nBin < 2) nBin = 2;
  // B2DEBUG(21, "tRange: [" << tRangeLow << "," << tRangeHigh << "], nBin: " << nBin);

  hist = TH1D("h_clsTime", "h_clsTime", nBin, tRangeLow, tRangeHigh);
  hist.GetXaxis()->SetLimits(tRangeLow, tRangeHigh);

  auto tdcTimes = m_inoEvent->getLeadingTDCs();

  for (int ij = 0; ij < totClusters; ij++) {
    double gCenter = tdcTimes[ij];
    double gSigma  = m_usedPars.clsSigma;
    // adding/filling a gauss to histogram
    addGausToHistogram(hist, 1., gCenter, gSigma, m_usedPars.fillSigmaN);
  }

} // end of createAndFillHistorgram


void TimeGroupingModule::searchGausPeaksInHistogram(TH1D& hist, std::vector<GroupInfo>& groupInfoVector)
{

  double maxPeak     = 0.;  //   height of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]
  double maxIntegral = 0.;  // integral of the highest peak in signal region [expectedSignalTimeMin, expectedSignalTimeMax]

  bool amDone = false;
  int  roughCleaningCounter = 0; // handle to take care when fit does not conserves
  while (!amDone) {

    // take the bin corresponding to the highest peak
    int    maxBin        = hist.GetMaximumBin();
    double maxBinCenter  = hist.GetBinCenter(maxBin);
    double maxBinContent = hist.GetBinContent(maxBin);

    // Set maxPeak for the first time
    if (maxPeak == 0 &&
        maxBinCenter > m_usedPars.expectedSignalTime[0] && maxBinCenter < m_usedPars.expectedSignalTime[2])
      maxPeak = maxBinContent;
    // we are done if the the height of the this peak is below threshold
    if (maxPeak != 0 && maxBinContent < maxPeak * m_usedPars.fracThreshold) { amDone = true; continue;}



    // preparing the gauss function for fitting the peak
    TF1 ngaus("ngaus", myGaus,
              hist.GetXaxis()->GetXmin(), hist.GetXaxis()->GetXmax(), 3);

    // setting the parameters according to the maxBinCenter and maxBinContnet
    double maxPar0 = maxBinContent * 2.50662827 * m_usedPars.fitRangeHalfWidth; // sqrt(2*pi) = 2.50662827
    ngaus.SetParameter(0, maxBinContent);
    ngaus.SetParLimits(0,
                       maxPar0 * 0.01,
                       maxPar0 * 2.);
    ngaus.SetParameter(1, maxBinCenter);
    ngaus.SetParLimits(1,
                       maxBinCenter - m_usedPars.fitRangeHalfWidth * 0.2,
                       maxBinCenter + m_usedPars.fitRangeHalfWidth * 0.2);
    ngaus.SetParameter(2, m_usedPars.fitRangeHalfWidth);
    ngaus.SetParLimits(2,
                       m_usedPars.limitSigma[0],
                       m_usedPars.limitSigma[1]);


    // fitting the gauss at the peak the in range [-fitRangeHalfWidth, fitRangeHalfWidth]
    int status = hist.Fit("ngaus", "NQ0", "",
                          maxBinCenter - m_usedPars.fitRangeHalfWidth,
                          maxBinCenter + m_usedPars.fitRangeHalfWidth);


    if (!status) {    // if fit converges

      double pars[3] = {
        ngaus.GetParameter(0),     // integral
        ngaus.GetParameter(1),     // center
        std::fabs(ngaus.GetParameter(2)) // sigma
      };

      // fit converges but parameters are at limit
      // Do a rough cleaning
      if (pars[2] <= m_usedPars.limitSigma[0] + 0.01 || pars[2] >= m_usedPars.limitSigma[1] - 0.01) {
        // subtract the faulty part from the histogram
        subtractGausFromHistogram(hist, maxPar0, maxBinCenter, m_usedPars.fitRangeHalfWidth, m_usedPars.removeSigmaN);
        if (roughCleaningCounter++ > m_usedPars.maxGroups) amDone = true;
        continue;
      }

      // Set maxIntegral for the first time
      if (maxPeak != 0 && maxIntegral == 0) maxIntegral = pars[0];
      // we are done if the the integral of the this peak is below threshold
      if (maxIntegral != 0 && pars[0] < maxIntegral * m_usedPars.fracThreshold) { amDone = true; continue;}


      // now subtract the fitted gaussian from the histogram
      subtractGausFromHistogram(hist, pars[0], pars[1], pars[2], m_usedPars.removeSigmaN);

      // store group information (integral, position, width)
      groupInfoVector.push_back(GroupInfo(pars[0], pars[1], pars[2]));
      // B2DEBUG(21, " group " << int(groupInfoVector.size())
      //         << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);

      if (int(groupInfoVector.size()) >= m_usedPars.maxGroups) { amDone = true; continue;}

    } else {    // fit did not converges
      // subtract the faulty part from the histogram
      subtractGausFromHistogram(hist, maxPar0, maxBinCenter, m_usedPars.fitRangeHalfWidth, m_usedPars.removeSigmaN);
      if (roughCleaningCounter++ > m_usedPars.maxGroups) amDone = true;
      continue;
    }
  }

} // end of searchGausPeaksInHistogram



void TimeGroupingModule::sortBackgroundGroups(std::vector<GroupInfo>& groupInfoVector)
{
  GroupInfo keyGroup;
  for (int ij = int(groupInfoVector.size()) - 2; ij >= 0; ij--) {
    keyGroup = groupInfoVector[ij];
    double keyGroupIntegral = std::get<0>(keyGroup);
    double keyGroupCenter = std::get<1>(keyGroup);
    bool isKeyGroupSignal = true;
    if (keyGroupIntegral != 0. &&
        (keyGroupCenter < m_usedPars.expectedSignalTime[0] || keyGroupCenter > m_usedPars.expectedSignalTime[2]))
      isKeyGroupSignal = false;
    if (isKeyGroupSignal) continue; // skip if signal

    int kj = ij + 1;
    while (kj < int(groupInfoVector.size())) {
      double otherGroupIntegral = std::get<0>(groupInfoVector[kj]);
      double otherGroupCenter = std::get<1>(groupInfoVector[kj]);
      bool isOtherGroupSignal = true;
      if (otherGroupIntegral != 0. &&
          (otherGroupCenter < m_usedPars.expectedSignalTime[0] || otherGroupCenter > m_usedPars.expectedSignalTime[2]))
        isOtherGroupSignal = false;
      if (!isOtherGroupSignal && (otherGroupIntegral > keyGroupIntegral)) break;
      groupInfoVector[kj - 1] = groupInfoVector[kj];
      kj++;
    }
    groupInfoVector[kj - 1] = keyGroup;
  }
}


void TimeGroupingModule::sortSignalGroups(std::vector<GroupInfo>& groupInfoVector)
{
  if (m_usedPars.signalLifetime > 0.) {
    GroupInfo keyGroup;
    for (int ij = 1; ij < int(groupInfoVector.size()); ij++) {
      keyGroup = groupInfoVector[ij];
      double keyGroupIntegral = std::get<0>(keyGroup);
      if (keyGroupIntegral <= 0) break;
      double keyGroupCenter = std::get<1>(keyGroup);
      bool isKeyGroupSignal = true;
      if (keyGroupIntegral > 0 &&
          (keyGroupCenter < m_usedPars.expectedSignalTime[0] || keyGroupCenter > m_usedPars.expectedSignalTime[2]))
        isKeyGroupSignal = false;
      if (!isKeyGroupSignal) break; // skip the backgrounds

      double keyWt = keyGroupIntegral * TMath::Exp(-std::fabs(keyGroupCenter - m_usedPars.expectedSignalTime[1]) /
                                                   m_usedPars.signalLifetime);
      int kj = ij - 1;
      while (kj >= 0) {
        double otherGroupIntegral = std::get<0>(groupInfoVector[kj]);
        double otherGroupCenter = std::get<1>(groupInfoVector[kj]);
        double grWt = otherGroupIntegral * TMath::Exp(-std::fabs(otherGroupCenter - m_usedPars.expectedSignalTime[1]) /
                                                      m_usedPars.signalLifetime);
        if (grWt > keyWt) break;
        groupInfoVector[kj + 1] = groupInfoVector[kj];
        kj--;
      }
      groupInfoVector[kj + 1] = keyGroup;
    }
  }
}


void TimeGroupingModule::assignGroupIdsToClusters(TH1D& hist, std::vector<GroupInfo>& groupInfoVector)
{
  int totClusters = m_inoEvent->getEntries();
  double tRangeLow  = hist.GetXaxis()->GetXmin();
  double tRangeHigh = hist.GetXaxis()->GetXmax();

  // assign all clusters groupId = -1 if no groups are found
  if (int(groupInfoVector.size()) == 0)
    for (auto hit : m_inoEvent->getHits()) {
      auto stripId = hit->stripId;
      m_inoEvent->setTimeGroupId(stripId).push_back(-1);
    }

  // loop over all the groups
  // some groups may be dummy, ie, (0,0,0). they are skipped
  for (int ij = 0; ij < int(groupInfoVector.size()); ij++) {

    double pars[3] = {
      std::get<0>(groupInfoVector[ij]),
      std::get<1>(groupInfoVector[ij]),
      std::get<2>(groupInfoVector[ij])
    };

    if (pars[2] == 0 && ij != int(groupInfoVector.size()) - 1) continue;
    // do not continue the last loop.
    // we assign the group Id to leftover clusters at the last loop.

    // for this group, accept the clusters falling within 5(default) sigma of group center
    double lowestAcceptedTime  = pars[1] - m_usedPars.acceptSigmaN * pars[2];
    double highestAcceptedTime = pars[1] + m_usedPars.acceptSigmaN * pars[2];
    if (lowestAcceptedTime < tRangeLow)   lowestAcceptedTime  = tRangeLow;
    if (highestAcceptedTime > tRangeHigh) highestAcceptedTime = tRangeHigh;
    // B2DEBUG(21, " group " << ij
    //         << " lowestAcceptedTime " << lowestAcceptedTime
    //         << " highestAcceptedTime " << highestAcceptedTime);

    // now loop over all the clusters to check which clusters fall in this range
    for (auto hit : m_inoEvent->getHits()) {
      auto stripId = hit->stripId;
      auto stripTimes = m_inoEvent->getCalibratedLeadingTimes(stripId);
      for (auto stripTime : stripTimes) {

        if (pars[2] != 0 &&   // if the last group is dummy, we straight go to leftover clusters
            stripTime >= lowestAcceptedTime && stripTime <= highestAcceptedTime) {

          // assigning groupId starting from 0
          m_inoEvent->setTimeGroupId(stripId).push_back(ij);

          // writing group info to clusters.
          // this is independent of group id, that means,
          if (m_usedPars.writeGroupInfo)
            m_inoEvent->setTimeGroupInfo(stripId).push_back(GroupInfo(pars[0], pars[1], pars[2]));

          // B2DEBUG(29, "   accepted cluster " << jk
          //         << " stripTime " << stripTime
          //         << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());

        } else {

          // B2DEBUG(29, "     rejected cluster " << jk
          //         << " stripTime " << stripTime);

          if (ij == int(groupInfoVector.size()) - 1 && // we are now at the last loop
              int(m_inoEvent->getTimeGroupId(stripId).size()) == 0) { // leftover clusters

            if (m_usedPars.includeOutOfRangeClusters && stripTime < tRangeLow)
              m_inoEvent->setTimeGroupId(stripId).push_back(m_usedPars.maxGroups + 1);  // underflow
            else if (m_usedPars.includeOutOfRangeClusters && stripTime > tRangeHigh)
              m_inoEvent->setTimeGroupId(stripId).push_back(m_usedPars.maxGroups + 2);  // overflow
            else
              m_inoEvent->setTimeGroupId(stripId).push_back(-1);               // orphan

            // B2DEBUG(29, "     leftover cluster " << jk
            //         << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());

          }
        }
      }
    } // end of loop over all clusters
  }   // end of loop over groups

}
