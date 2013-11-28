/******************************************************************/
/**
 * @file	testApp.cpp
 * @brief	Example for ofxKinectNui addon
 * @note
 * @todo
 * @bug	
 *
 * @author	sadmb
 * @date	Oct. 28, 2011
 */
/******************************************************************/
#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	ofxKinectNui::InitSetting initSetting;
	initSetting.grabVideo = false;
	initSetting.grabDepth = true;
	initSetting.grabAudio = false;
	initSetting.grabLabel = true;
	initSetting.grabSkeleton = false;
	initSetting.grabCalibratedVideo = false;
	initSetting.grabLabelCv = true;
	kinect.init(initSetting);
//	kinect.init(false, true, false, true, false, false, true, false);	/// only use depth capturing and separated label capturing
	kinect.open();
	//kinect.setAngle(-5);
	
	ofSetVerticalSync(true);

	//set defaults
	bAutoSetBase = false;
	bShowHelp = false;
	bHorizontalMirror = false;
	bVerticalMirror = false;
	scale = 1;
	layerDensity = 1;

	// get current status
	angle = kinect.getCurrentAngle();
	width = kinect.getDepthResolutionWidth();
	height = kinect.getDepthResolutionHeight();
	
	// allocate image sizes
	depthImage.allocate(width, height);
	blImage.allocate(width, height);
	rgbImage.allocate(width, height);
	/*thresholdedImage.allocate(320, 240);
	threshold = 20;
	*/
	
	/*colorImage.allocate(320, 240, OF_IMAGE_COLOR_ALPHA);
	labelImages = new ofxCvGrayscaleImage[ofxKinectNui::KINECT_PLAYERS_INDEX_NUM];
	for(int i = 0; i < ofxKinectNui::KINECT_PLAYERS_INDEX_NUM; i++){
		labelImages[i].allocate(320, 240);
	}
	contourFinders = new ofxCvContourFinder[ofxKinectNui::KINECT_PLAYERS_INDEX_NUM - 1]; /// we get 7 players in maximum.
	*/
	settings.loadFile("settings.xml");
	nearClipping = settings.getValue("KINECT:CLIPPING:NEAR", kinect.getNearClippingDistance());
	farClipping = settings.getValue("KINECT:CLIPPING:FAR", kinect.getFarClippingDistance());
	kinect.setFarClippingDistance(farClipping);
	kinect.setNearClippingDistance(nearClipping);

	ofSetFrameRate(60);

	// init base pixels
	deleteBase();
	
}

//--------------------------------------------------------------
void testApp::update() {
	kinect.update();
	if(kinect.isOpened()){
		depthImage.setFromPixels(kinect.getDepthPixels());
		/*thresholdedImage = depthImage;
		thresholdedImage.threshold(threshold);
		contourFinderDepth.findContours(thresholdedImage, 20, (320*240)/3, 10, true);
		*/
		////// You can skip copying to thresholdedImage if you don't need depth draw
		// depthImage.threshold(threshold);
		// contourFinderDepth.findContours(depthImage, 20, (320*240)/3, 10, true);

		/* added code for my rgb colour image */
		rgbImage.setFromPixels(kinect.getDepthPixels());

		if (bAutoSetBase)
			setBase(0.8);
			
		unsigned char * pix = depthImage.getPixels();
		unsigned char * blpix = blImage.getPixels();

		int numPixels = depthImage.getWidth() * depthImage.getHeight();
		for(int i = 0; i < numPixels; i++) {
			int delta = abs(pix[i]-blpix[i])/layerDensity;
			if ( delta >= 3 ) 
				pix[i] = delta;
			else
				pix[i] = 0;
		}
		
		cvCvtColor(depthImage.getCvImage(), rgbImage.getCvImage(), CV_GRAY2BGR);

		//Grayscale to RGB conversion similar to Photoshops RGB Mode to Grayscale Mode conversion
		//app. 30% Red, 59% Green, 11% Blue 
		int w = rgbImage.width;  
		int h = rgbImage.height;  
		int bpp = 3;  
		unsigned char * pixels = rgbImage.getPixels();  
  
		for (int i = 0; i < w; i++){  
			for (int j = 0; j < h; j++){
					pixels[(j*w+i)*bpp+0] = pixels[(j*w+i)*bpp+0] * (0.3*255);  
					pixels[(j*w+i)*bpp+1] = pixels[(j*w+i)*bpp+1] * (0.59*255);  
					pixels[(j*w+i)*bpp+2] = pixels[(j*w+i)*bpp+2] * (0.11*255);  
				  
			}  
		}
		
		/* color image with labels */
		/*colorImage.setFromPixels(kinect.getLabelPixels());
		for(int i = 0; i < ofxKinectNui::KINECT_PLAYERS_INDEX_NUM; i++){
			labelImages[i].setFromPixels(kinect.getLabelPixelsCv(i));
			if(i > 0){
				contourFinders[i - 1].findContours(labelImages[i], 20, (320*240)/3, 10, true);
			}
		}*/
	}
}

//--------------------------------------------------------------
void testApp::draw() {
	ofBackground(100, 100, 100);
	//depthImage.draw(20, 20);		/// normal depth images
	//thresholdedImage.draw(360, 20);	/// thresholded depth images
	/*HWND hWnd;
	PRECT lpRect;
	GetWindowRect(hWnd, lpRect);
	rgbImage.draw(0, 0, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);*/
	rgbImage.mirror(bVerticalMirror, bHorizontalMirror);
	//rgbImage.scale(scale,scale);
	//rgbImage.translate(1-scale,1-scale);
	rgbImage.transform(0, width/2, height/2, scale, scale, 0, 0);
	rgbImage.draw(0, 0, ofGetWidth(), ofGetHeight());

	/*ofSetColor(0);
	ofFill();
	ofRect(700, 20, 320, 240);
	ofRect(700, 280, 320, 240);
	ofRect(20, 280, 320, 240);
	ofSetColor(255);
	
	ofEnableAlphaBlending();
	colorImage.draw(20, 280);		/// normal label images
	ofDisableAlphaBlending();
	labelImages[0].draw(360, 280);	/// whole players' silhouette
	
	// contours from depth images
	for(int i = 0; i < contourFinderDepth.nBlobs; i++){
		contourFinderDepth.blobs[i].draw(700, 20);
	}

	// contours from label images
	for(int i = 0; i < ofxKinectNui::KINECT_PLAYERS_INDEX_NUM - 1; i++){
		for(int j = 0; j < contourFinders[i].nBlobs; j++){
			contourFinders[i].blobs[j].draw(700, 280);
		}
	}
	*/
	// draw instructions
	ofSetColor(255, 255, 255);
	stringstream reportStream;
	if (bShowHelp) {
		reportStream << " (press: < >), fps: " << ofGetFrameRate() << endl
					 << "press 'c' to close the stream and 'o' to open it again, stream is: " << kinect.isOpened() << endl
					 << "press 'a' to turn auto-baseline setting, auto-baseline is: " << bAutoSetBase << endl
					 << "press '0' to set the baseline on the depth" << endl
					 << "press '9' to delete the baseline" << endl
					 << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
					 << "press LEFT and RIGHT to change the far clipping distance: " << farClipping << " mm" << endl
					 << "press 'f' to flip image horizontally" << endl
					 << "press 'v' to flip image vertically" << endl
					 << "press '+' and '-' to change the near clipping distance: " << nearClipping << " mm" << endl
					 << "press 'z' and 'x' to change the scaling: " << scale << endl
					 << "press 'q' and 'w' to change the layer density: " << layerDensity << " px" << endl
					 << "press 's' to save the near/ far clipping distance to xml file." << endl
					 << "press 'h' to turn help text on or off" << endl;
		ofDrawBitmapString(reportStream.str(), 20, ofGetHeight() - 190);
	}
	else {
		reportStream << "press 'h' to turn help text on or off" << endl;
		ofDrawBitmapString(reportStream.str(), 20, ofGetHeight() - 30);
	}
	
}


//--------------------------------------------------------------
void testApp::exit() {
	//kinect.setAngle(0);
	kinect.close();
}


//--------------------------------------------------------------
void testApp::setBase(double ratio) {
	// ratio must be less or equal to one
	ofxCvGrayscaleImage tempImage;
	tempImage.allocate(width,height);
	tempImage.setFromPixels(kinect.getDepthPixels());

	unsigned char * blpix = blImage.getPixels();
	unsigned char * tmpix = tempImage.getPixels();

		int numPixels = rgbImage.getWidth() * rgbImage.getHeight();
		for(int i = 0; i < numPixels; i++) {
			blpix[i]  = tmpix[i]*(1-ratio) + blpix[i]*ratio;
		}
}

//--------------------------------------------------------------
void testApp::resetBase() {
	blImage.setFromPixels(kinect.getDepthPixels());
}

void testApp::deleteBase() {
	unsigned char * blpix = blImage.getPixels();
	int numPixels = rgbImage.getWidth() * rgbImage.getHeight();
	for(int i = 0; i < numPixels; i++) {
		blpix[i]  = 0;
	}
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch(key){
	case 'o':
	case 'O':
		kinect.open();
		break;
	case 'c':
	case 'C':
		kinect.close();
		break;
	case 'a':
	case 'A':
		bAutoSetBase = !bAutoSetBase;
		break;
	case 'h':
	case 'H':
		bShowHelp = !bShowHelp;
		break;
	case 'f':
	case 'F':
		bHorizontalMirror = !bHorizontalMirror;
		break;
	case 'v':
	case 'V':
		bVerticalMirror = !bVerticalMirror;
		break;
	case'0':
		resetBase();
		break;
	case'9':
		deleteBase();
		break;
	case OF_KEY_UP:
		angle++;
		if(angle > 27){
			angle = 27;
		}
		kinect.setAngle(angle);
		break;
	case OF_KEY_DOWN:
		angle--;
		if(angle < -27){
			angle = -27;
		}
		kinect.setAngle(angle);
		break;
	case OF_KEY_LEFT:
		if(farClipping > nearClipping + 10){
			farClipping -= 10;
			kinect.setFarClippingDistance(farClipping);
		}
		break;
	case OF_KEY_RIGHT:
		if(farClipping < 4000){
			farClipping += 10;
			kinect.setFarClippingDistance(farClipping);
		}
		break;
	case '-':
		if(nearClipping >= 10){
			nearClipping -= 10;
			kinect.setNearClippingDistance(nearClipping);
		}
		break;
	case '+':
		if(nearClipping < farClipping - 10){
			nearClipping += 10;
			kinect.setNearClippingDistance(nearClipping);
		}
		break;
	case 'q':
	case 'Q':
		if(layerDensity > 1){
			layerDensity -= 1;
		}
		break;
	case 'w':
	case 'W':
		if(layerDensity < 10){
			layerDensity += 1;
		}
		break;
	case 'z':
	case 'Z':
		if(scale > 0.1){
			scale = scale - 0.01;
		}
		break;
	case 'x':
	case 'X':
		if(scale < 10){
			scale = scale + 0.01;
		}
		break;
	case 's':
	case 'S':
		settings.setValue("KINECT:CLIPPING:NEAR", nearClipping);
		settings.setValue("KINECT:CLIPPING:FAR", farClipping);
		settings.saveFile("settings.xml");
		break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

