#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup() {

	kinect.init();
	//kinect.init(true);  // shows infrared instead of RGB video image
	//kinect.init(false, false);  // disable infrared/rgb video iamge (faster fps)
	kinect.setVerbose(true);
	kinect.open();
	
	// start with the live kinect source
	kinectSource = &kinect;

	colorImage.allocate(kinect.width, kinect.height);
	blImage.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	outputImage.allocate(kinect.width, kinect.height);

	nearThreshold = 200;
	farThreshold  = 180;
	bThreshWithOpenCV = true;
	zoom = 0;
	baseColor = 255;
	
	ofSetFrameRate(60);

	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);

	// init base pixels
	setBase(255);
}

//--------------------------------------------------------------
void testApp::update() {

	ofBackground(100, 100, 100);
	
	kinectSource->update();
	
	// there is a new frame and we are connected
	if(kinectSource->isFrameNew()) {
	

		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinectSource->getDepthPixels(), kinect.width, kinect.height);
		colorImage.setFromPixels(kinectSource->getDepthPixels(), kinect.width, kinect.height);

		// we do two thresholds - one for the far plane and one for the near plane
		// we then do a cvAnd to get the pixels which are a union of the two thresholds 
/*		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;	
			grayThreshNear.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		};
*/
		// update the cv images
		grayImage.flagImageChanged();
		colorImage.flagImageChanged();

		unsigned char * pix = grayImage.getPixels();
		unsigned char * blpix = blImage.getPixels();

		int numPixels = grayImage.getWidth() * grayImage.getHeight();
		for(int i = 0; i < numPixels; i++) {
			int delta = abs(pix[i]-blpix[i]);
			if ( delta >= 1 ) 
				pix[i] = delta;
			else
				pix[i] = 0;
		}
		
		cvCvtColor(grayImage.getCvImage(), colorImage.getCvImage(), CV_GRAY2BGR);

		//Grayscale to RGB conversion similar to Photoshops RGB Mode to Grayscale Mode conversion
		//app. 30% Red, 59% Green, 11% Blue 
		int w = colorImage.width;  
		int h = colorImage.height;  
		int bpp = 3;  
		unsigned char * pixels = colorImage.getPixels();  
  
		for (int i = 0; i < w; i++){  
			for (int j = 0; j < h; j++){
					pixels[(j*w+i)*bpp+0] = pixels[(j*w+i)*bpp+0] * (0.3*255);  
					pixels[(j*w+i)*bpp+1] = pixels[(j*w+i)*bpp+1] * (0.59*255);  
					pixels[(j*w+i)*bpp+2] = pixels[(j*w+i)*bpp+2] * (0.11*255);  
				  
			}  
		}
		
	}
}

//--------------------------------------------------------------
void testApp::draw() {
	ofSetColor(255, 255, 255);
	colorImage.draw(0, 0, 1024, 768);
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
	kinectPlayer.close();
	kinectRecorder.close();
}

//--------------------------------------------------------------
void testApp::setBase(int bcolor) {
	blImage.setFromPixels(kinectSource->getDepthPixels(), kinect.width, kinect.height);
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		case ' ':
			bThreshWithOpenCV = !bThreshWithOpenCV;
		break;
		
		case'0':
			setBase(255);
			break;
	
		case'p':
			bDrawPointCloud = !bDrawPointCloud;
			break;
	
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
			
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'z':		
			zoom ++;							// zoom in
			if (zoom < 0) zoom = 0;
			break;
			
		case 'x':		
			zoom --;							// zoom out
			if (zoom < 0) zoom = 0;
			break;
			
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
			
		case 'o':
			kinect.setCameraTiltAngle(angle);	// go back to prev tilt
			kinect.open();
			break;
			
		case 'c':
			kinect.setCameraTiltAngle(0);		// zero the tilt
			kinect.close();
			break;
			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;

		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}

