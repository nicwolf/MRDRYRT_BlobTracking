#pragma once

#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "ofxOpenCv.h"
#include "ofxDatGui.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void setupGui();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		int width, height;

		ofxKFW2::Device kinect;
		int KINECT_DEPTH_WIDTH  =  512;
		int KINECT_DEPTH_HEIGHT =  424;
		int KINECT_COLOR_WIDTH  = 1920;
		int KINECT_COLOR_HEIGHT = 1080;
		struct KinectSettings {
			float nearPlane = 0.0;
			float farPlane = 0.08;
			bool bClipNear;
			float nearClip = 0.0;
			bool bClipFar;
			float farClip = 0.05;
			bool bClipLeft = false;
			float nearLeftClip = 0.0;
			float farLeftClip = 0.23;
			bool bClipRight = false;
			float nearRightClip = 1.0;
			float farRightClip = 0.74;
			bool bClipTop = false;
			float nearTopClip = 0.0;
			float farTopClip = 0.41;
			bool bClipBottom = false;
			float nearBottomClip = 1.0;
			float farBottomClip = 0.81;
		} kinectSettings;

		ofxOscSender oscSender;
		struct OscSettings {
			string hostname = "localhost";
			int port = 12345;
		} oscSettings;

		ofxCvGrayscaleImage depthDifferenceImage;
		ofFbo depthBackgroundFbo;
		ofFbo depthIncomingFbo;
		ofFbo depthDifferenceFbo;
		ofFbo depthBlurFbo;
		ofShader depthPreprocessShader;
		ofShader depthDifferenceShader;
		ofShader depthBlurShader;
		ofMesh depthMesh;
		ofxCvContourFinder contourFinder;
		struct ContourFinderSettings {
			float threshold = 0.03;
			int minBlobSize = 500;
			int maxBlobSize = 691200;
			int maxNumBlobs = 10;
			bool bFindHoles = false;
			bool bLearnBackground = true;
			bool bBlurPass = false;
			float blurStrength = 1.0;
		} contourFinderSettings;

		// Gui Elements
		ofxDatGui* gui;
		// -- Header/Footer Elements
		ofxDatGuiHeader* guiHeader;
		ofxDatGuiFooter* guiFooter;
		// -- Monitoring Elements
		ofxDatGuiFRM* guiFRM;
		// -- Kinect GUI Elements
		ofxDatGuiButton* buttonLearnBackground;
		ofxDatGuiSlider* sliderNearPlane;
		ofxDatGuiSlider* sliderFarPlane;
		ofxDatGuiToggle* toggleClipNear;
		ofxDatGuiSlider* sliderNearClip;
		ofxDatGuiToggle* toggleClipFar;
		ofxDatGuiSlider* sliderFarClip;
		ofxDatGuiToggle* toggleClipLeft;
		ofxDatGuiSlider* sliderNearLeftClip;
		ofxDatGuiSlider* sliderFarLeftClip;
		ofxDatGuiToggle* toggleClipRight;
		ofxDatGuiSlider* sliderNearRightClip;
		ofxDatGuiSlider* sliderFarRightClip;
		ofxDatGuiToggle* toggleClipTop;
		ofxDatGuiSlider* sliderNearTopClip;
		ofxDatGuiSlider* sliderFarTopClip;
		ofxDatGuiToggle* toggleClipBottom;
		ofxDatGuiSlider* sliderNearBottomClip;
		ofxDatGuiSlider* sliderFarBottomClip;
		// -- Blob Tracking GUI Elements
		ofxDatGuiSlider* sliderThreshold;
		ofxDatGuiToggle* toggleBlurPass;
		ofxDatGuiSlider* sliderBlurStrength;
		ofxDatGuiSlider* sliderMinBlobSize;
		ofxDatGuiSlider* sliderMaxBlobSize;
		ofxDatGuiSlider* sliderMaxNumBlobs;
		ofxDatGuiToggle* toggleFindHoles;
		// -- Network Settings Elements
		ofxDatGuiTextInput* inputOscHostname;
		ofxDatGuiTextInput* inputOscPort;
};
