#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	kinect.open();
	kinect.initColorSource();
	kinect.initDepthSource();

	colorImage.allocate(KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT);
	
	grayImage.allocate(KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT);
	grayBackground.allocate(KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT);
	grayDifference.allocate(KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT);

	depthImage.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);
	depthBackground.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);
	depthDifference.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);

	testShader.load("shaders/test.vert", "shaders/test.frag");
	depthMesh.addVertex(ofPoint(-1.0, -1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(0.0, KINECT_DEPTH_HEIGHT));
	depthMesh.addVertex(ofPoint( 1.0,  1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(KINECT_DEPTH_WIDTH, 0.0));
	depthMesh.addVertex(ofPoint(-1.0,  1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(0.0, 0.0));
	depthMesh.addVertex(ofPoint( 1.0, -1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT));
	depthMesh.addIndex(0);
	depthMesh.addIndex(1);
	depthMesh.addIndex(2);
	depthMesh.addIndex(0);
	depthMesh.addIndex(3);
	depthMesh.addIndex(1);
	depthMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	depthFbo.allocate(512, 424);
	depthFbo.begin();
	ofClear(255, 255, 255, 255);
	depthFbo.end();

	bLearnBackground = true;
	threshold = 80;

	testImage.load("images/disambiguate_before.png");

}

//--------------------------------------------------------------
void ofApp::update(){
	kinect.update();
	if (kinect.isFrameNew()) {
		// TODO: Will we need to scale depth values? Perhaps the user sets near and
		//       far planes and the depth value is mapped from (val, near, far, 0, 255)
		//       and clamped (val, 0, 255).

		// Process Color Image
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
void ofApp::draw() {
	//img.draw(0, 0);
	//colorImage.draw(0, 0);
	//kinect.getColorSource()->draw(0, 0, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT);
	/*ofSetHexColor(0xffffff);
	for (int i = 0; i < contourFinder.nBlobs; ++i) {
	contourFinder.blobs[i].draw(0, 0);
	}*/
	float r = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
	depthFbo.begin();
	testShader.begin();
	testShader.setUniformTexture("uColorTex", kinect.getDepthSource()->getTexture(), 1);
	depthMesh.draw();
	//kinect.getColorSource()->draw(0, 0, 512, 424);
	testShader.end();
	depthFbo.end();
	depthFbo.draw(0, 0, 512, 424);

	// TODO: Deal with FBO Flipping Issue
	// TODO: Use Depth Map for Blob Tracking
	// TODO: Implement thresholding/differencing in pre-processing shader
	// TODO: Implement blob tracking with depth map
	// TODO: Add a GUI for updating bg, changing threshold, changing clipping settings,
	//       blob sizes, etc...
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
