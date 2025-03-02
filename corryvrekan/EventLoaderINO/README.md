---
# SPDX-FileCopyrightText: 2023-2024 CERN and the Corryvreckan authors
# SPDX-License-Identifier: CC-BY-4.0 OR MIT
---
# EventLoaderHDF5
**Maintainer**: Suryanarayan Mondal (<suryamondal@gmail.com>)
**Module Type**: *DETECTOR*  
**Status**: Work in progress

### Description
This module loads data from mICAL root files

### Parameters
* `filename`: Input file name.
* `timestamp_shift`: Shift the timestamp of the record by the defined value in nanoseconds.
* `detectorRegion`: Part of the detector to be aligned, given in X and Y strip ranges

### Plots produced

The following plots are produced:

* 2D map of pixel positions
* Histogram with pixel charge

### Usage
```toml
[EventLoaderINO]
filename = "path/to/file"
detectorRegion = [[0,63],[0,63]]
```
