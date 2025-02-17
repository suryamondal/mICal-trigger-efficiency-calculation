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

TTimeStamp MAX_TIME(2018, 12, 31, 0, 0, 0, 0, true);
TTimeStamp MIN_TIME(2018, 12, 10, 0, 0, 0, 0, true);

int main(int argc, char *argv[]) {

  INO::INOCalibrationManager& instance = INO::INOCalibrationManager::getInstance();
  double starttime = MIN_TIME.AsDouble();
  double endtime = MAX_TIME.AsDouble();
  for (int l = 0; l < 10 ; l++)
    for (int axis = 0; axis < 2; axis++)
      for (int s = 0; s < 64; s++)
        instance.setStripTimeOffset({0, 0, 0, l, axis, s}, starttime, endtime, -255);

  return 0;
}
