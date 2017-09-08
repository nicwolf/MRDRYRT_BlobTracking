#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);

	setupGui();

	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();

	oscSender.setup(oscSettings.hostname, oscSettings.port);

	depthDifferenceImage.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT);

	depthPreprocessShader.load("shaders/depthMap.vert", "shaders/depthPreprocess.frag");
	depthDifferenceShader.load("shaders/depthMap.vert", "shaders/depthDifference.frag");
	depthBlurShader.load("shaders/depthMap.vert", "shaders/depthBlur.frag");

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

	depthIncomingFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthIncomingFbo.begin();
		ofClear(ofColor::white);
	depthIncomingFbo.end();

	depthDifferenceFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthDifferenceFbo.begin();
		ofClear(ofColor::white);
	depthDifferenceFbo.end();

	depthBackgroundFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthBackgroundFbo.begin();
		ofClear(ofColor::white);
	depthBackgroundFbo.end();

	depthBlurFbo.allocate(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, GL_RGB);
	depthBlurFbo.begin();
		ofClear(ofColor::white);
	depthBlurFbo.end();

	width = ofGetWidth();
	height = ofGetHeight();

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();
	if (kinect.isFrameNew()) {

		// Scale Depth Values
		// ------------------
		// TODO: Would blurring make this more effective?
		// TODO: Rework how clipping is done.
		depthIncomingFbo.begin();
			depthPreprocessShader.begin();
				depthPreprocessShader.setUniform1f("near", kinectSettings.nearPlane);
				depthPreprocessShader.setUniform1f("far", kinectSettings.farPlane);
				depthPreprocessShader.setUniform1i("bClipNear", kinectSettings.bClipNear);
				depthPreprocessShader.setUniform1f("nearClip", kinectSettings.nearClip);
				depthPreprocessShader.setUniform1i("bClipFar", kinectSettings.bClipFar);
				depthPreprocessShader.setUniform1f("farClip", kinectSettings.farClip);
				depthPreprocessShader.setUniform1i("bClipLeft", kinectSettings.bClipLeft);
				depthPreprocessShader.setUniform1f("nearLeftClip", kinectSettings.nearLeftClip);
				depthPreprocessShader.setUniform1f("farLeftClip", kinectSettings.farLeftClip);
				depthPreprocessShader.setUniform1i("bClipRight", kinectSettings.bClipRight);
				depthPreprocessShader.setUniform1f("nearRightClip", kinectSettings.nearRightClip);
				depthPreprocessShader.setUniform1f("farRightClip", kinectSettings.farRightClip);
				depthPreprocessShader.setUniform1i("bClipTop", kinectSettings.bClipTop);
				depthPreprocessShader.setUniform1f("nearTopClip", kinectSettings.nearTopClip);
				depthPreprocessShader.setUniform1f("farTopClip", kinectSettings.farTopClip);
				depthPreprocessShader.setUniform1i("bClipBottom", kinectSettings.bClipBottom);
				depthPreprocessShader.setUniform1f("nearBottomClip", kinectSettings.nearBottomClip);
				depthPreprocessShader.setUniform1f("farBottomClip", kinectSettings.farBottomClip);
				depthPreprocessShader.setUniformTexture("uDepthTex", kinect.getDepthSource()->getTexture(), 1);
				depthMesh.draw();
			depthPreprocessShader.end();
		depthIncomingFbo.end();

		// Blur Passes
		// -----------
		if (contourFinderSettings.bBlurPass) {
			// Horizontal Pass
			depthBlurFbo.begin();
				depthBlurShader.begin();
					depthBlurShader.setUniform1f("blurStrength", contourFinderSettings.blurStrength);
					depthBlurShader.setUniform1i("bHorizontalPass", 1);
					depthBlurShader.setUniformTexture("uDepthTex", depthIncomingFbo.getTexture(), 1);
					depthMesh.draw();
				depthBlurShader.end();
			depthBlurFbo.end();
			// Vertical Pass
			depthIncomingFbo.begin();
				depthBlurShader.begin();
					depthBlurShader.setUniform1f("blurStrength", contourFinderSettings.blurStrength);
					depthBlurShader.setUniform1i("bHorizontalPass", 0);
					depthBlurShader.setUniformTexture("uDepthTex", depthBlurFbo.getTexture(), 1);
					depthMesh.draw();
				depthBlurShader.end();
			depthIncomingFbo.end();
		}

		// Learn Background
		// ----------------
		if (contourFinderSettings.bLearnBackground) {
			depthBackgroundFbo.begin();
				depthIncomingFbo.draw(0, 0, 512, 424);
			depthBackgroundFbo.end();
			contourFinderSettings.bLearnBackground = false;
		}

		// Difference and Threshold Image
		// ------------------------------
		depthDifferenceFbo.begin();
			depthDifferenceShader.begin();
				depthDifferenceShader.setUniform1f("threshold", contourFinderSettings.threshold);
				depthDifferenceShader.setUniformTexture("uDepthBackgroundTex", depthBackgroundFbo.getTexture(), 1);
				depthDifferenceShader.setUniformTexture("uDepthIncomingTex", depthIncomingFbo.getTexture(), 2);
				depthMesh.draw();
			depthDifferenceShader.end();
		depthDifferenceFbo.end();

		// Find Blobs
		// ----------
		ofPixels depthDifferencePixels;
		depthDifferenceFbo.readToPixels(depthDifferencePixels);
		depthDifferenceImage.setFromPixels(depthDifferencePixels.getChannel(0));
		contourFinder.findContours(
			depthDifferenceImage,
			contourFinderSettings.minBlobSize,
			contourFinderSettings.maxBlobSize,
			contourFinderSettings.maxNumBlobs,
			contourFinderSettings.bFindHoles
		);

		// Send Blob Data via OSC
		// ----------------------
		ofxOscMessage m;
		m.setAddress("/blobTracker/nBlobs");
		m.addIntArg(contourFinder.nBlobs);
		oscSender.sendMessage(m, false);
		// TODO: Send Blob Information
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	// Draw the Color Feed
	kinect.getColorSource()->draw(0, 0, width / 2, height / 2);
	// Draw the Depth Difference
	depthDifferenceFbo.draw(width / 2, height / 2, width / 2, height / 2);
	// Draw the Depth Background
	depthBackgroundFbo.draw(0, height / 2, width / 2, height / 2);
	// Draw the Contours into the Incoming Depth Feed...
	depthIncomingFbo.begin();
		ofSetHexColor(0xffffff);
		for (int i = 0; i < contourFinder.nBlobs; ++i) {
			contourFinder.blobs[i].draw(0, 0);
		}
	depthIncomingFbo.end();
	// ... And Draw the Feed to Screen
	depthIncomingFbo.draw(width / 2, 0, width / 2, height / 2);
}

//--------------------------------------------------------------
void ofApp::setupGui() {
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
	sliderNearPlane = folderKinectSetup->addSlider(":: Near", 0.0, 1.0, kinectSettings.nearPlane);
	sliderNearPlane->bind(kinectSettings.nearPlane);
	sliderFarPlane = folderKinectSetup->addSlider(":: Far", 0.0, 1.0, kinectSettings.farPlane);
	sliderFarPlane->bind(kinectSettings.farPlane);
	toggleClipNear = folderKinectSetup->addToggle(":: Clip Near");
	toggleClipNear->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipNear = e.checked;
	});
	toggleClipNear->setChecked(kinectSettings.bClipLeft);
	sliderNearClip = folderKinectSetup->addSlider(":: Near Clip", 0.0, 1.0, kinectSettings.nearClip);
	sliderNearClip->bind(kinectSettings.nearClip);
	toggleClipFar = folderKinectSetup->addToggle(":: Clip Far");
	toggleClipFar->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipFar = e.checked;
	});
	toggleClipFar->setChecked(kinectSettings.bClipLeft);
	sliderFarClip = folderKinectSetup->addSlider(":: Far Clip", 0.0, 1.0, kinectSettings.farClip);
	sliderFarClip->bind(kinectSettings.farClip);
	toggleClipLeft = folderKinectSetup->addToggle(":: Clip Left");
	toggleClipLeft->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipLeft = e.checked;
	});
	toggleClipLeft->setChecked(kinectSettings.bClipLeft);
	sliderNearLeftClip = folderKinectSetup->addSlider(":: Near Left Clip", 0.0, 1.0, kinectSettings.nearLeftClip);
	sliderNearLeftClip->bind(kinectSettings.nearLeftClip);
	sliderFarLeftClip = folderKinectSetup->addSlider(":: Far Left Clip", 0.0, 1.0, kinectSettings.farLeftClip);
	sliderFarLeftClip->bind(kinectSettings.farLeftClip);
	toggleClipRight = folderKinectSetup->addToggle(":: Clip Right");
	toggleClipRight->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipRight = e.checked;
	});
	toggleClipRight->setChecked(kinectSettings.bClipRight);
	sliderNearRightClip = folderKinectSetup->addSlider(":: Near Right Clip", 0.0, 1.0, kinectSettings.nearRightClip);
	sliderNearRightClip->bind(kinectSettings.nearRightClip);
	sliderFarRightClip = folderKinectSetup->addSlider(":: Far Right Clip", 0.0, 1.0, kinectSettings.farRightClip);
	sliderFarRightClip->bind(kinectSettings.farRightClip);
	toggleClipTop = folderKinectSetup->addToggle(":: Clip Top");
	toggleClipTop->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipTop = e.checked;
	});
	toggleClipTop->setChecked(kinectSettings.bClipTop);
	sliderNearTopClip = folderKinectSetup->addSlider(":: Near Top Clip", 0.0, 1.0, kinectSettings.nearTopClip);
	sliderNearTopClip->bind(kinectSettings.nearTopClip);
	sliderFarTopClip = folderKinectSetup->addSlider(":: Far Top Clip", 0.0, 1.0, kinectSettings.farTopClip);
	sliderFarTopClip->bind(kinectSettings.farTopClip);
	toggleClipBottom = folderKinectSetup->addToggle(":: Clip Bottom");
	toggleClipBottom->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		kinectSettings.bClipBottom = e.checked;
	});
	toggleClipBottom->setChecked(kinectSettings.bClipBottom);
	sliderNearBottomClip = folderKinectSetup->addSlider(":: Near Bottom Clip", 0.0, 1.0, kinectSettings.nearBottomClip);
	sliderNearBottomClip->bind(kinectSettings.nearBottomClip);
	sliderFarBottomClip = folderKinectSetup->addSlider(":: Far Bottom Clip", 0.0, 1.0, kinectSettings.farBottomClip);
	sliderFarBottomClip->bind(kinectSettings.farBottomClip);
	gui->addBreak()->setHeight(10.0f);

	// Add Blob Tracking Folder
	ofxDatGuiFolder* folderBlobTrackingParameters = gui->addFolder("Blob Tracking", ofColor::blue);
	buttonLearnBackground = folderBlobTrackingParameters->addButton(":: Learn Background");
	buttonLearnBackground->onButtonEvent([&](ofxDatGuiButtonEvent e) {
		contourFinderSettings.bLearnBackground = true;
	});
	toggleBlurPass = folderBlobTrackingParameters->addToggle(":: Blur Pass");
	toggleBlurPass->setChecked(contourFinderSettings.bBlurPass);
	toggleBlurPass->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		contourFinderSettings.bBlurPass = e.checked;
	});
	sliderBlurStrength = folderBlobTrackingParameters->addSlider(":: Blur Strength", 0.0, 25.0, contourFinderSettings.blurStrength);
	sliderBlurStrength->bind(contourFinderSettings.blurStrength);
	sliderThreshold = folderBlobTrackingParameters->addSlider(":: Threshold", 0.0, 1.0, contourFinderSettings.threshold);
	sliderThreshold->bind(contourFinderSettings.threshold);
	sliderMinBlobSize = folderBlobTrackingParameters->addSlider(":: Min Size (Hundred px)", 0, 9999, contourFinderSettings.minBlobSize/100);
	sliderMinBlobSize->onSliderEvent([&](ofxDatGuiSliderEvent e) {
		contourFinderSettings.minBlobSize = e.value * 100;
	});
	sliderMaxBlobSize = folderBlobTrackingParameters->addSlider(":: Max Size (Hundred px)", 0, 9999, contourFinderSettings.maxBlobSize/100);
	sliderMaxBlobSize->onSliderEvent([&](ofxDatGuiSliderEvent e) {
		contourFinderSettings.maxBlobSize = e.value * 100;
	});
	sliderMaxNumBlobs = folderBlobTrackingParameters->addSlider(":: Max No. Blobs", 0, 20, contourFinderSettings.maxNumBlobs);
	sliderMaxNumBlobs->bind(contourFinderSettings.maxNumBlobs);
	sliderMaxNumBlobs->setPrecision(0);
	toggleFindHoles = folderBlobTrackingParameters->addToggle(":: Find Holes");
	toggleFindHoles->setChecked(contourFinderSettings.bFindHoles);
	toggleFindHoles->onToggleEvent([&](ofxDatGuiToggleEvent e) {
		contourFinderSettings.bFindHoles = e.checked;
	});
	gui->addBreak()->setHeight(10.0f);

	// Add Networking Folder
	ofxDatGuiFolder* folderNetworkSettings = gui->addFolder("OSC Settings", ofColor::red);
	inputOscHostname = folderNetworkSettings->addTextInput("Hostname", oscSettings.hostname);
	inputOscHostname->setTextUpperCase(false);
	inputOscHostname->onTextInputEvent([&](ofxDatGuiTextInputEvent e) {
		oscSettings.hostname = e.text;
		oscSender.setup(oscSettings.hostname, oscSettings.port);
	});
	inputOscPort = folderNetworkSettings->addTextInput("Port", to_string(oscSettings.port));
	inputOscPort->setInputType(ofxDatGuiInputType::NUMERIC);
	inputOscPort->onTextInputEvent([&](ofxDatGuiTextInputEvent e) {
		oscSettings.port = stoi(e.text);
		oscSender.setup(oscSettings.hostname, oscSettings.port);
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
	width = w;
	height = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
