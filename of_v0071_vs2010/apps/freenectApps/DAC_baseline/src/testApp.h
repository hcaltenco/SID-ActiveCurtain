#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxVectorMath.h"
#include "ofAppRunner.h"

class testApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
		void drawPointCloud();

		void setBase(int bcolor);
		void keyPressed(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		
		ofxKinect 			kinect;
		ofxKinectRecorder 	kinectRecorder;
		ofxKinectPlayer 	kinectPlayer;
		
		/// used to switch between the live kinect and the recording player
		ofxBase3DVideo* 	kinectSource;

		ofxCvColorImage		colorImage;
		ofxCvGrayscaleImage	blImage;			// baseline image
		ofxCvGrayscaleImage grayImage;			// grayscale depth image
		ofxCvGrayscaleImage grayThreshNear;		// the near thresholded image
		ofxCvGrayscaleImage grayThreshFar;		// the far thresholded image
		ofxCvGrayscaleImage	outputImage;		// temp crop image

		bool				bThreshWithOpenCV;
		bool				bDrawPointCloud;

		int 				nearThreshold;
		int					farThreshold;

		int					angle;
		int					zoom;	
		int					baseColor;

		unsigned char		* bpix;
};
