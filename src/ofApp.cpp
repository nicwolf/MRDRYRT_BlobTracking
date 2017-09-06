#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	kinect.open();
	kinect.initColorSource();
	img.allocate(1920, 1080, OF_IMAGE_COLOR);
	colorImage.allocate(1920, 1080);
	grayImage.allocate(1920, 1080);
	grayBackground.allocate(1920, 1080);
	grayDifference.allocate(1920, 1080);
	bLearnBackground = true;
	threshold = 80;
}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();
	if (kinect.isFrameNew()) {
		// TODO: Implement Blob Tracking for this grayscale image.
		// TODO: Check to see if ofxCvFloatImage is useful for this. Will it preserve
		//       0-1 values?
		// TODO: Will we need to scale depth values? Perhaps the user sets near and
		//       far planes and the depth value is mapped from (val, near, far, 0, 255)
		//       and clamped (val, 0, 255). 
		colorImage = kinect.getColorSource()->getPixels();
		ofPixels test = kinect.getColorSource()->getPixels();
		grayImage.setFromPixels(test.getChannel(0));
		if (bLearnBackground) {
			grayBackground = grayImage;
			bLearnBackground = false;
		}
		grayDifference.absDiff(grayBackground, grayImage);
		grayDifference.threshold(threshold);
		contourFinder.findContours(grayDifference, 20, 691200, 10, false);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	//img.draw(0, 0);
	//colorImage.draw(0, 0);
	kinect.getColorSource()->draw(0, 0, 1920, 1080);
	ofSetHexColor(0xffffff);
	for (int i = 0; i < contourFinder.nBlobs; ++i) {
		contourFinder.blobs[i].draw(0, 0);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
