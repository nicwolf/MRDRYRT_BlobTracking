#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	setupGui();

	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();

	depthImage.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);
	depthBackground.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);
	depthDifference.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);
	depthDifference.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);

	depthScaleShader.load("shaders/depthMap.vert", "shaders/depthScale.frag");
	depthDiffShader.load("shaders/depthMap.vert", "shaders/depthDifference.frag");

	depthMesh.addVertex(ofPoint(-1.0, -1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(0.0, 0.0));
	depthMesh.addVertex(ofPoint( 1.0,  1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT));
	depthMesh.addVertex(ofPoint(-1.0,  1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(0.0, KINECT_DEPTH_HEIGHT));
	depthMesh.addVertex(ofPoint( 1.0, -1.0, 0.0));
	depthMesh.addTexCoord(ofVec2f(KINECT_DEPTH_WIDTH, 0.0));
	depthMesh.addIndex(0);
	depthMesh.addIndex(1);
	depthMesh.addIndex(2);
	depthMesh.addIndex(0);
	depthMesh.addIndex(3);
	depthMesh.addIndex(1);
	depthMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	depthScaleFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthScaleFbo.begin();
		ofClear(ofColor::white);
	depthScaleFbo.end();

	depthDiffFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthDiffFbo.begin();
		ofClear(ofColor::white);
	depthDiffFbo.end();

	bLearnBackground = true;
	threshold = 80;

	testImage.allocate(512, 424, OF_IMAGE_COLOR);

	width = ofGetWidth();
	height = ofGetHeight();

}

//--------------------------------------------------------------
void ofApp::update(){
	width = ofGetWidth();
	height = ofGetHeight();
	kinect.update();
	if (kinect.isFrameNew()) {

		// Scale Depth Values
		// ------------------
		// TODO: Would blurring make this more effective?
		// TODO: Rework how clipping is done.
		depthScaleFbo.begin();
			depthScaleShader.begin();
				depthScaleShader.setUniform1f("near", kinectSetupParameters.nearPlane);
				depthScaleShader.setUniform1f("far", kinectSetupParameters.farPlane);
				depthScaleShader.setUniform1i("bClipNear", kinectSetupParameters.bClipNear);
				depthScaleShader.setUniform1f("nearClip", kinectSetupParameters.nearClip);
				depthScaleShader.setUniform1i("bClipFar", kinectSetupParameters.bClipFar);
				depthScaleShader.setUniform1f("farClip", kinectSetupParameters.farClip);
				depthScaleShader.setUniform1i("bClipLeft", kinectSetupParameters.bClipLeft);
				depthScaleShader.setUniform1f("nearLeftClip", kinectSetupParameters.nearLeftClip);
				depthScaleShader.setUniform1f("farLeftClip", kinectSetupParameters.farLeftClip);
				depthScaleShader.setUniform1i("bClipRight", kinectSetupParameters.bClipRight);
				depthScaleShader.setUniform1f("nearRightClip", kinectSetupParameters.nearRightClip);
				depthScaleShader.setUniform1f("farRightClip", kinectSetupParameters.farRightClip);
				depthScaleShader.setUniform1i("bClipTop", kinectSetupParameters.bClipTop);
				depthScaleShader.setUniform1f("nearTopClip", kinectSetupParameters.nearTopClip);
				depthScaleShader.setUniform1f("farTopClip", kinectSetupParameters.farTopClip);
				depthScaleShader.setUniform1i("bClipBottom", kinectSetupParameters.bClipBottom);
				depthScaleShader.setUniform1f("nearBottomClip", kinectSetupParameters.nearBottomClip);
				depthScaleShader.setUniform1f("farBottomClip", kinectSetupParameters.farBottomClip);
				depthScaleShader.setUniformTexture("uDepthTex", kinect.getDepthSource()->getTexture(), 1);
				depthMesh.draw();
			depthScaleShader.end();
		depthScaleFbo.end();

		// Learn Background
		// ----------------
		// TODO: Can we speed this up by keeping the background in an FBO?
		if (bLearnBackground) {
			ofPixels depthBackgroundPixels;
			depthScaleFbo.readToPixels(depthBackgroundPixels);
			testImage.setFromPixels(depthBackgroundPixels);
			bLearnBackground = false;
		}

		// Difference and Threshold Image
		// ------------------------------
		depthDiffFbo.begin();
			depthDiffShader.begin();
				depthDiffShader.setUniform1f("threshold", blobTrackingParameters.threshold);
				depthDiffShader.setUniformTexture("uDepthBackgroundTex", testImage.getTexture(), 1);
				depthDiffShader.setUniformTexture("uDepthIncomingTex", depthScaleFbo.getTexture(), 2);
				depthMesh.draw();
			depthDiffShader.end();
		depthDiffFbo.end();

		// Find Blobs
		// ----------
		ofPixels depthDifferencePixels;
		depthDiffFbo.readToPixels(depthDifferencePixels);
		depthDifference.setFromPixels(depthDifferencePixels.getChannel(0));
		contourFinder.findContours(
			depthDifference,
			blobTrackingParameters.minBlobSize,
			blobTrackingParameters.maxBlobSize,
			blobTrackingParameters.maxNumBlobs,
			blobTrackingParameters.bFindHoles
		);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	kinect.getColorSource()->draw(0, 0, width / 2, height / 2);
	depthDiffFbo.draw(width / 2, height / 2, width / 2, height / 2);
	testImage.draw(0, height / 2, width / 2, height / 2);
	depthScaleFbo.begin();
		ofSetHexColor(0xffffff);
		for (int i = 0; i < contourFinder.nBlobs; ++i) {
			contourFinder.blobs[i].draw(0, 0);
		}
	depthScaleFbo.end();
	depthScaleFbo.draw(width / 2, 0, width / 2, height / 2);

}

//--------------------------------------------------------------
void ofApp::setupGui() {
	// TODO: Add Networking

	// Initialize GUI
	gui = new ofxDatGui(ofxDatGuiAnchor::TOP_LEFT);
	gui->setWidth(400.00, 150.0);

	// Add Header
	guiHeader = gui->addHeader(":: Blob Tracking System :: ");
	guiFooter = gui->addFooter();
	gui->addBreak()->setHeight(10.0f);

	// Add Monitoring
	guiFRM = gui->addFRM(1.0f);
	gui->addBreak()->setHeight(10.0f);

	// Add Kinect Setup Folder
	ofxDatGuiFolder* folderKinectSetup = gui->addFolder("Kinect Setup");
	sliderNearPlane = folderKinectSetup->addSlider(":: Near", 0.0, 1.0, kinectSetupParameters.nearPlane);
	sliderNearPlane->bind(kinectSetupParameters.nearPlane);
	sliderFarPlane = folderKinectSetup->addSlider(":: Far", 0.0, 1.0, kinectSetupParameters.farPlane);
	sliderFarPlane->bind(kinectSetupParameters.farPlane);
	toggleClipNear = folderKinectSetup->addToggle(":: Clip Near");
	toggleClipNear->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipNear = e.checked;
	});
	toggleClipNear->setChecked(kinectSetupParameters.bClipLeft);
	sliderNearClip = folderKinectSetup->addSlider(":: Near Clip", 0.0, 1.0, kinectSetupParameters.nearClip);
	sliderNearClip->bind(kinectSetupParameters.nearClip);
	toggleClipFar = folderKinectSetup->addToggle(":: Clip Far");
	toggleClipFar->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipFar = e.checked;
	});
	toggleClipFar->setChecked(kinectSetupParameters.bClipLeft);
	sliderFarClip = folderKinectSetup->addSlider(":: Far Clip", 0.0, 1.0, kinectSetupParameters.farClip);
	sliderFarClip->bind(kinectSetupParameters.farClip);
	toggleClipLeft = folderKinectSetup->addToggle(":: Clip Left");
	toggleClipLeft->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipLeft = e.checked;
	});
	toggleClipLeft->setChecked(kinectSetupParameters.bClipLeft);
	sliderNearLeftClip = folderKinectSetup->addSlider(":: Near Left Clip", 0.0, 1.0, kinectSetupParameters.nearLeftClip);
	sliderNearLeftClip->bind(kinectSetupParameters.nearLeftClip);
	sliderFarLeftClip = folderKinectSetup->addSlider(":: Far Left Clip", 0.0, 1.0, kinectSetupParameters.farLeftClip);
	sliderFarLeftClip->bind(kinectSetupParameters.farLeftClip);
	toggleClipRight = folderKinectSetup->addToggle(":: Clip Right");
	toggleClipRight->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipRight = e.checked;
	});
	toggleClipRight->setChecked(kinectSetupParameters.bClipRight);
	sliderNearRightClip = folderKinectSetup->addSlider(":: Near Right Clip", 0.0, 1.0, kinectSetupParameters.nearRightClip);
	sliderNearRightClip->bind(kinectSetupParameters.nearRightClip);
	sliderFarRightClip = folderKinectSetup->addSlider(":: Far Right Clip", 0.0, 1.0, kinectSetupParameters.farRightClip);
	sliderFarRightClip->bind(kinectSetupParameters.farRightClip);
	toggleClipTop = folderKinectSetup->addToggle(":: Clip Top");
	toggleClipTop->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipTop = e.checked;
	});
	toggleClipTop->setChecked(kinectSetupParameters.bClipTop);
	sliderNearTopClip = folderKinectSetup->addSlider(":: Near Top Clip", 0.0, 1.0, kinectSetupParameters.nearTopClip);
	sliderNearTopClip->bind(kinectSetupParameters.nearTopClip);
	sliderFarTopClip = folderKinectSetup->addSlider(":: Far Top Clip", 0.0, 1.0, kinectSetupParameters.farTopClip);
	sliderFarTopClip->bind(kinectSetupParameters.farTopClip);
	toggleClipBottom = folderKinectSetup->addToggle(":: Clip Bottom");
	toggleClipBottom->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSetupParameters.bClipBottom = e.checked;
	});
	toggleClipBottom->setChecked(kinectSetupParameters.bClipBottom);
	sliderNearBottomClip = folderKinectSetup->addSlider(":: Near Bottom Clip", 0.0, 1.0, kinectSetupParameters.nearBottomClip);
	sliderNearBottomClip->bind(kinectSetupParameters.nearBottomClip);
	sliderFarBottomClip = folderKinectSetup->addSlider(":: Far Bottom Clip", 0.0, 1.0, kinectSetupParameters.farBottomClip);
	sliderFarBottomClip->bind(kinectSetupParameters.farBottomClip);
	gui->addBreak()->setHeight(10.0f);

	// Add Blob Tracking Folder
	ofxDatGuiFolder* folderBlobTrackingParameters = gui->addFolder("Blob Tracking", ofColor::blue);
	buttonLearnBackground = folderBlobTrackingParameters->addButton(":: Learn Background");
	buttonLearnBackground->onButtonEvent([&](ofxDatGuiButtonEvent e) {
		bLearnBackground = true;
	});
	sliderThreshold = folderBlobTrackingParameters->addSlider(":: Threshold", 0.0, 1.0, blobTrackingParameters.threshold);
	sliderThreshold->bind(blobTrackingParameters.threshold);
	sliderMinBlobSize = folderBlobTrackingParameters->addSlider(":: Min Size (Hundred px)", 0, 9999, blobTrackingParameters.minBlobSize/100);
	sliderMinBlobSize->onSliderEvent([&](ofxDatGuiSliderEvent e) {
		blobTrackingParameters.minBlobSize = e.value * 100;
	});
	sliderMaxBlobSize = folderBlobTrackingParameters->addSlider(":: Max Size (Hundred px)", 0, 9999, blobTrackingParameters.maxBlobSize/100);
	sliderMaxBlobSize->onSliderEvent([&](ofxDatGuiSliderEvent e) {
		blobTrackingParameters.maxBlobSize = e.value * 100;
	});
	sliderMaxNumBlobs = folderBlobTrackingParameters->addSlider(":: Max No. Blobs", 0, 20, blobTrackingParameters.maxNumBlobs);
	sliderMaxNumBlobs->bind(blobTrackingParameters.maxNumBlobs);
	sliderMaxNumBlobs->setPrecision(0);
	toggleFindHoles = folderBlobTrackingParameters->addToggle(":: Find Holes");
	toggleFindHoles->setChecked(blobTrackingParameters.bFindHoles);
	toggleFindHoles->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		blobTrackingParameters.bFindHoles = e.checked;
	});
	gui->addBreak()->setHeight(10.0f);
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
