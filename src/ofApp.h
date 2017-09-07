#pragma once

#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
		
		ofxKFW2::Device kinect;
		int KINECT_DEPTH_WIDTH  =  512;
		int KINECT_DEPTH_HEIGHT =  424;
		int KINECT_COLOR_WIDTH  = 1920;
		int KINECT_COLOR_HEIGHT = 1080;

		int colorWidth, colorHeight;
		ofxCvColorImage colorImage;
		ofxCvGrayscaleImage grayImage;
		ofxCvGrayscaleImage grayBackground;
		ofxCvGrayscaleImage grayDifference;
		ofxCvGrayscaleImage depthImage;
		ofxCvGrayscaleImage depthBackground;
		ofxCvGrayscaleImage depthDifference;
		ofxCvContourFinder contourFinder;

		ofFbo depthFbo;
		ofShader testShader;

		ofMesh depthMesh;

		ofImage testImage;

		int threshold;
		bool bLearnBackground;

};
