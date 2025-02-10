
#pragma once

// std
#include <string>
#include <memory>

// root
#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>

#include "INOEvent.h"


namespace INO {

  /**
   * structure containing the relevant information
   * of TimeGrouping module
   */
  struct TimeGroupingParameters {
    /** Expected range of svd time histogram [ns]. */
    Float_t tRange[2];
    /** Time bin width is 1/rebinningFactor [ns]. */
    Int_t   rebinningFactor;
    /** Number of Gaussian sigmas used to fill the time histogram for each cluster. */
    Float_t fillSigmaN;
    /** Limit of cluster time sigma for the fit for the peak-search [ns]. */
    Float_t limitSigma[2];
    /** Half width of the range in which the fit for the peak-search is performed [ns]. */
    Float_t fitRangeHalfWidth;
    /** Remove upto this sigma of fitted gauss from histogram. */
    Float_t removeSigmaN;
    /** Minimum fraction of candidates in a peak (wrt to the highest peak) considered for fitting in the peak-search. */
    Float_t fracThreshold;
    /** maximum number of groups to be accepted. */
    Int_t   maxGroups;
    /** Expected time-range and mean of the signal [ns]. (min, center, max) */
    Float_t expectedSignalTime[3];
    /** Group prominence is weighted with exponential weight with a lifetime defined by this parameter [ns]. */
    Float_t signalLifetime;
    /** Number of groups expected to contain the signal clusters. */
    Int_t   numberOfSignalGroups;
    /** Assign groupID = 0 to all clusters belonging to the signal groups. */
    Bool_t  formSingleSignalGroup;
    /** Clusters are tagged within this of fitted group. */
    Float_t acceptSigmaN;
    /** Write group info in Cluster, otherwise kept empty. */
    Bool_t  writeGroupInfo;
    /** Assign groups to under and overflow. */
    Bool_t  includeOutOfRangeClusters;
    /**
     * Cls-time resolution based on sensor side and type,
     * types -> 0: L3, 1: Barrel, 2: Forward.
     * sides -> 0: V, 1: U,
     * vector elements are sigmas wrt cls-size (currently from 1 to 6).
     */
    double clsSigma;
  };

  /*! typedef to be used to store Gauss parameters (integral, center, sigma) */
  typedef std::tuple<double, double, double> GroupInfo;

  /**
   * Imports Clusters of the detector and converts them to spacePoints.
   */
  class INOTimeGroupingModule {

  public:

    /** Constructor */
    INOTimeGroupingModule(std::shared_ptr<INOEvent> data);

    /** EventWise jobs
     * Grouping of Clusters is performed here
     */
    void process();

  protected:

    /**
     * hits.
     */
    std::shared_ptr<INOEvent> m_inoEvent = std::make_shared<INOEvent>();

    /**
     * module parameter values used.
     */
    TimeGroupingParameters m_usedPars;

    // helper functions

    /** Create Histogram and Fill cluster time in it
     *
     * 1. Optimize the range of the histogram and declare it
     * 2. fill the histogram shaping each cluster with a normalised gaussian G(cluster time, resolution)
     */
    void createAndFillHistorgram(TH1D& hist);

    /** Find Gaussian components in a Histogram
     *
     * 1. Highest peak is found
     * 2. It is fitted in a given range
     * 3. Fit parameters are stored
     * 4. Gauss peak is removed from histogram
     * 5. Process is repeated until a few criteria are met.
     */
    void searchGausPeaksInHistogram(TH1D& hist, std::vector<GroupInfo>& groupInfoVector);

    /*! increase the size of vector to max, this helps in sorting */
    void resizeToMaxSize(std::vector<GroupInfo>& groupInfoVector)
    {
      groupInfoVector.resize(m_usedPars.maxGroups, GroupInfo(0., 0., 0.));
    }

    /** Sort Background Groups
     *
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     * The probability of being background is max at groupID = 19 and increases with group number decreasing.
     */
    void sortBackgroundGroups(std::vector<GroupInfo>& groupInfoVector);

    /** Sort Signals
     *
     * Sorting signal groups based on exponential weight.
     * This decreases chance of near-signal bkg groups getting picked.
     * The probability of being signal is max at groupID = 0 and decreases with group number increasing.
     */
    void sortSignalGroups(std::vector<GroupInfo>& groupInfoVector);

    /** Assign groupId to the clusters
     *
     * 1. Looped over all the groups
     * 2. Acceptance time-range is computed using the center and m_acceptSigmaN
     * 3. Looped over all the clusters
     * 4. Clusters in the range is assigned the respective groupId
     */
    void assignGroupIdsToClusters(TH1D& hist, std::vector<GroupInfo>& groupInfoVector);

  };


  /*! Gauss function to be used in the fit. */
  inline double myGaus(const double* x, const double* par)
  {
    return par[0] * TMath::Gaus(x[0], par[1], par[2], true);
  }


  /** Add (or Subtract) a Gaussian to (or from) a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void addGausToHistogram(TH1D& hist,
                                 const double& integral, const double& center, const double& sigma,
                                 const double& sigmaN, const bool& isAddition = true)
  {
    int startBin = hist.FindBin(center - sigmaN * sigma);
    int   endBin = hist.FindBin(center + sigmaN * sigma);
    if (startBin < 1) startBin = 1;
    if (endBin > (hist.GetNbinsX())) endBin = hist.GetNbinsX();

    for (int ijx = startBin; ijx <= endBin; ijx++) {
      double tbinc       = hist.GetBinCenter(ijx);
      double tbincontent = hist.GetBinContent(ijx);

      if (isAddition) tbincontent += integral * TMath::Gaus(tbinc, center, sigma, true);
      else tbincontent -= integral * TMath::Gaus(tbinc, center, sigma, true);

      hist.SetBinContent(ijx, tbincontent);
    }
  }


  /**  Subtract a Gaussian from a histogram
   *
   * The gauss is calculated upto the sigmaN passed to the function.
   */
  inline void subtractGausFromHistogram(TH1D& hist,
                                        const double& integral, const double& center, const double& sigma,
                                        const double& sigmaN)
  {
    addGausToHistogram(hist, integral, center, sigma, sigmaN, false);
  }

} // end namespace Belle2
