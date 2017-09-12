# MRDRYRT Blob Tracking

This is blob tracking software for use in the MRDRYRT project.

### Tested With

* Windows 10

### Requirements

* OpenFrameworks v0.9.8
  - ofxKinectForWindows2
  - ofxOpenCv
  - ofxOSC
  - ofxDatGui

## Description of OSC

This application sends tracking data via a network connection using the OSC protocol. It sends the following information:

* `/blobTracker/nBlobs`
  - The number of currently found blobs.
* `/blobTracker/blob/n/area`
  - The area of blob `n` as a percentage of the Kinect's depth camera's total field of view.
* `/blobTracker/blob/n/centroid/x`
  - The x-coordinate of blob `n` in normalized [0, 1] image space.
* `/blobTracker/blob/n/centroid/y`
  - The y-coordinate of blob `n` in normalized [0, 1] image space.
