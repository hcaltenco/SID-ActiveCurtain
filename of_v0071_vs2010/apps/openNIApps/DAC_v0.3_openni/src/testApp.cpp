#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup() {
/*	isLive			= false;
	isTracking		= false;
	isTrackingHands	= false;
	isFiltering		= false;
	isRecording		= false;
	isCloud			= false;
	isCPBkgnd		= true;
	isMasking		= false;
*/
	isBaselineSet = false;

	nearThreshold = 500;
	farThreshold  = 1000;

//	filterFactor = 0.1f;

	setupRecording();
	
	ofBackground(0, 0, 0);

}

void testApp::setupRecording(string _filename) {

#if defined (TARGET_OSX) //|| defined(TARGET_LINUX) // only working on Mac/Linux at the moment (but on Linux you need to run as sudo...)
	hardware.setup();				// libusb direct control of motor, LED and accelerometers
	hardware.setLedOption(LED_OFF); // turn off the led just for yacks (or for live installation/performances ;-)
#endif

	recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
	//recordContext.setupUsingXMLFile();
	recordDepth.setup(&recordContext);
//	recordImage.setup(&recordContext);
	recordDepth.setDepthColoring(COLORING_PSYCHEDELIC);

/*	recordUser.setup(&recordContext);
	recordUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	recordUser.setUseMaskPixels(isMasking);
	recordUser.setUseCloudPoints(isCloud);
	recordUser.setMaxNumberOfUsers(2);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)

	recordHandTracker.setup(&recordContext, 4);
	recordHandTracker.setSmoothing(filterFactor);		// built in openni hand track smoothing...
	recordHandTracker.setFilterFactors(filterFactor);	// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with
*/
	recordContext.toggleRegisterViewport();
	recordContext.toggleMirror();

//	oniRecorder.setup(&recordContext, ONI_STREAMING);
	//oniRecorder.setup(&recordContext, ONI_CYCLIC, 60);
	//read the warning in ofxOpenNIRecorder about memory usage with ONI_CYCLIC recording!!!

	baseDepth.setup(&recordContext);
	baseDepth.setDepthColoring(COLORING_PSYCHEDELIC);
	
}
/*
void testApp::setupPlayback(string _filename) {

	playContext.shutdown();
	playContext.setupUsingRecording(ofToDataPath(_filename));
	playDepth.setup(&playContext);
	playImage.setup(&playContext);

	playUser.setup(&playContext);
	playUser.setSmoothing(filterFactor);				// built in openni skeleton smoothing...
	playUser.setUseMaskPixels(isMasking);
	playUser.setUseCloudPoints(isCloud);

	playHandTracker.setup(&playContext, 4);
	playHandTracker.setSmoothing(filterFactor);			// built in openni hand track smoothing...
	playHandTracker.setFilterFactors(filterFactor);		// custom smoothing/filtering (can also set per hand with setFilterFactor)...set them all to 0.1f to begin with

	playContext.toggleRegisterViewport();
	playContext.toggleMirror();

}
*/
//--------------------------------------------------------------
void testApp::update(){

#ifdef TARGET_OSX // only working on Mac at the moment
	hardware.update();
#endif
	
	recordContext.update();
	
	if (!isBaselineSet) {
		baseDepth.update();
		depthBaseMask.setFromPixels(baseDepth.getDepthPixels(nearThreshold, farThreshold),
			baseDepth.getWidth(), baseDepth.getHeight(), OF_IMAGE_GRAYSCALE);

		isBaselineSet = true;
	}

	recordDepth.update();

	depthRangeMask.setFromPixels(recordDepth.getDepthPixels(nearThreshold, farThreshold),
		recordDepth.getWidth(), recordDepth.getHeight(), OF_IMAGE_GRAYSCALE);
	
	unsigned char * pix = depthRangeMask.getPixels();
	unsigned char * blpix = depthBaseMask.getPixels();
	
	int numPixels = recordDepth.getWidth() * recordDepth.getHeight();
	for(int i = 0; i < numPixels; i++) {
		int delta = abs(pix[i]-blpix[i]);
		if ( delta >= 3 ) 
			pix[i] = delta;
		else
			pix[i] = 0;
	}

	bwDepthImage.setFromPixels(pix, recordDepth.getWidth(), recordDepth.getHeight());
	colorDepthImage.setFromPixels(pix, recordDepth.getWidth(), recordDepth.getHeight());
	
	cvCvtColor(bwDepthImage.getCvImage(), colorDepthImage.getCvImage(), CV_GRAY2BGR);
	
	//Grayscale to RGB conversion similar to Photoshops RGB Mode to Grayscale Mode conversion
	//app. 30% Red, 59% Green, 11% Blue 
	int w = colorDepthImage.getWidth();  
	int h = colorDepthImage.getHeight(); 
	int bpp = 3;  
	unsigned char * pixels = colorDepthImage.getPixels();

	for (int i = 0; i < w; i++){  
		for (int j = 0; j < h; j++){
			pixels[(j*w+i)*bpp+0] = pixels[(j*w+i)*bpp+0] * (0.3*255);  
			pixels[(j*w+i)*bpp+1] = pixels[(j*w+i)*bpp+1] * (0.59*255);  
			pixels[(j*w+i)*bpp+2] = pixels[(j*w+i)*bpp+2] * (0.11*255);  

		}  
	}


}

//--------------------------------------------------------------
void testApp::draw(){

	ofSetColor(255, 255, 255);

	//glPushMatrix();
	//glScalef(0.75, 0.75, 0.75);

	//recordDepth.draw(0,0,640,480);
	//baseDepth.draw(640, 0, 640, 480);
	
	colorDepthImage.draw(0, 0, 1024, 768);
	//depthBaseMask.draw(640, 0, 640, 480);

	//glPopMatrix();

	ofSetColor(255, 255, 0);

	string statusBaseline	= (string)(isBaselineSet ? "BASELINE SET" : "BASELINE NOT SET");
/*	string statusRec		= (string)(!isRecording ? "READY" : "RECORDING");
	string statusSkeleton	= "";//(string)(isTracking ? "TRACKING USERS: " + (string)(isLive ? ofToString(recordUser.getNumberOfTrackedUsers()) : ofToString(playUser.getNumberOfTrackedUsers())) + "" : "NOT TRACKING USERS");
	string statusSmoothSkel = "";//(string)(isLive ? ofToString(recordUser.getSmoothing()) : ofToString(playUser.getSmoothing()));
	string statusHands		= "";//(string)(isTrackingHands ? "TRACKING HANDS: " + (string)(isLive ? ofToString(recordHandTracker.getNumTrackedHands()) : ofToString(playHandTracker.getNumTrackedHands())) + ""  : "NOT TRACKING");
	string statusFilter		= (string)(isFiltering ? "FILTERING" : "NOT FILTERING");
	string statusFilterLvl	= "";//ofToString(filterFactor);
	string statusSmoothHand = "";//(string)(isLive ? ofToString(recordHandTracker.getSmoothing()) : ofToString(playHandTracker.getSmoothing()));
	string statusMask		= (string)(!isMasking ? "HIDE" : (isTracking ? "SHOW" : "YOU NEED TO TURN ON TRACKING!!"));
	string statusCloud		= (string)(isCloud ? "ON" : "OFF");
	string statusCloudData	= (string)(isCPBkgnd ? "SHOW BACKGROUND" : (isTracking ? "SHOW USER" : "YOU NEED TO TURN ON TRACKING!!"));
*/
	string statusHardware;

#ifdef TARGET_OSX // only working on Mac at the moment
	ofPoint statusAccelerometers = hardware.getAccelerometers();
	stringstream	statusHardwareStream;

	statusHardwareStream
	<< "ACCELEROMETERS:"
	<< " TILT: " << hardware.getTiltAngle() << "/" << hardware.tilt_angle
	<< " x - " << statusAccelerometers.x
	<< " y - " << statusAccelerometers.y
	<< " z - " << statusAccelerometers.z;

	statusHardware = statusHardwareStream.str();
#endif

	stringstream msg;

	msg
//	<< "    s : start/stop recording  : " << statusRec << endl
	<< "    0 : set new baseline	  : " << statusBaseline << endl
/*	<< "    t : skeleton tracking     : " << statusSkeleton << endl
	<< "( / ) : smooth skely (openni) : " << statusSmoothSkel << endl
	<< "    h : hand tracking         : " << statusHands << endl
	<< "    f : filter hands (custom) : " << statusFilter << endl
	<< "[ / ] : filter hands factor   : " << statusFilterLvl << endl
	<< "; / ' : smooth hands (openni) : " << statusSmoothHand << endl
	<< "    m : drawing masks         : " << statusMask << endl
	<< "    c : draw cloud points     : " << statusCloud << endl
	<< "    b : cloud user data       : " << statusCloudData << endl
*/	<< "- / + : nearThreshold         : " << ofToString(nearThreshold) << endl
	<< "< / > : farThreshold          : " << ofToString(farThreshold) << endl
	<< endl
//	<< "File  : " << oniRecorder.getCurrentFileName() << endl
	<< "FPS   : " << ofToString(ofGetFrameRate()) << "  " << statusHardware << endl;

	ofDrawBitmapString(msg.str(), 20, 700);

}
/*
void testApp:: drawMasks() {
	glPushMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
	allUserMasks.draw(640, 0, 640, 480);
	glDisable(GL_BLEND);
    glPopMatrix();
	user1Mask.draw(320, 480, 320, 240);
	user2Mask.draw(640, 480, 320, 240);
	
}

void testApp::drawPointCloud(ofxUserGenerator * user_generator, int userID) {

	glPushMatrix();

	int w = user_generator->getWidth();
	int h = user_generator->getHeight();

	glTranslatef(w, h/2, -500);
	ofRotateY(pointCloudRotationY);

	glBegin(GL_POINTS);

	int step = 1;

	for(int y = 0; y < h; y += step) {
		for(int x = 0; x < w; x += step) {
			ofPoint pos = user_generator->getWorldCoordinateAt(x, y, userID);
			if (pos.z == 0 && isCPBkgnd) continue;	// gets rid of background -> still a bit weird if userID > 0...
			ofColor color = user_generator->getWorldColorAt(x,y, userID);
			glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	}

	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);

	glPopMatrix();
}

*/
//--------------------------------------------------------------
void testApp::keyPressed(int key){

	float smooth;

	switch (key) {
#ifdef TARGET_OSX // only working on Mac at the moment
		case 357: // up key
			hardware.setTiltAngle(hardware.tilt_angle++);
			break;
		case 359: // down key
			hardware.setTiltAngle(hardware.tilt_angle--);
			break;
#endif
/*		case 's':
		case 'S':
			if (isRecording) {
				oniRecorder.stopRecord();
				isRecording = false;
				break;
			} else {
				oniRecorder.startRecord(generateFileName());
				isRecording = true;
				break;
			}
			break;
		case 't':
		case 'T':
			isTracking = !isTracking;
			break;
		case 'h':
		case 'H':
			isTrackingHands = !isTrackingHands;
			if(isLive) recordHandTracker.toggleTrackHands();
			if(!isLive) playHandTracker.toggleTrackHands();
			break;
		case 'f':
		case 'F':
			isFiltering = !isFiltering;
			recordHandTracker.isFiltering = isFiltering;
			playHandTracker.isFiltering = isFiltering;
			break;
		case 'm':
		case 'M':
			isMasking = !isMasking;
			recordUser.setUseMaskPixels(isMasking);
			playUser.setUseMaskPixels(isMasking);
			break;
		case 'c':
		case 'C':
			isCloud = !isCloud;
			recordUser.setUseCloudPoints(isCloud);
			playUser.setUseCloudPoints(isCloud);
			break;
		case 'b':
		case 'B':
			isCPBkgnd = !isCPBkgnd;
			break;
		case '9':
		case '(':
			smooth = recordUser.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordUser.setSmoothing(smooth - 0.1f);
				playUser.setSmoothing(smooth - 0.1f);
			}
			break;
		case '0':
		case ')':
			smooth = recordUser.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordUser.setSmoothing(smooth + 0.1f);
				playUser.setSmoothing(smooth + 0.1f);
			}
			break;
		case '[':
		//case '{':
			if (filterFactor - 0.1f > 0.0f) {
				filterFactor = filterFactor - 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ']':
		//case '}':
			if (filterFactor + 0.1f <= 1.0f) {
				filterFactor = filterFactor + 0.1f;
				recordHandTracker.setFilterFactors(filterFactor);
				if (oniRecorder.getCurrentFileName() != "") playHandTracker.setFilterFactors(filterFactor);
			}
			break;
		case ';':
		case ':':
			smooth = recordHandTracker.getSmoothing();
			if (smooth - 0.1f > 0.0f) {
				recordHandTracker.setSmoothing(smooth -  0.1f);
				playHandTracker.setSmoothing(smooth -  0.1f);
			}
			break;
		case '\'':
		case '\"':
			smooth = recordHandTracker.getSmoothing();
			if (smooth + 0.1f <= 1.0f) {
				recordHandTracker.setSmoothing(smooth +  0.1f);
				playHandTracker.setSmoothing(smooth +  0.1f);
			}
			break;
*/		
		case '0':
			if (isBaselineSet) {
			//	setupPlayback(oniRecorder.getCurrentFileName());
				isBaselineSet = false;
			} else {
				isBaselineSet = true;
			}
			break;
		case '>':
		case '.':
			farThreshold += 50;
			if (farThreshold > recordDepth.getMaxDepth()) farThreshold = recordDepth.getMaxDepth();
			break;
		case '<':
		case ',':
			farThreshold -= 50;
			if (farThreshold < 0) farThreshold = 0;
			break;

		case '+':
			nearThreshold += 50;
			if (nearThreshold > recordDepth.getMaxDepth()) nearThreshold = recordDepth.getMaxDepth();
			break;

		case '-':
		case '_':
			nearThreshold -= 50;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'r':
			recordContext.toggleRegisterViewport();
			break;
		default:
			break;
	}
}

/*string testApp::generateFileName() {

	string _root = "kinectRecord";

	string _timestamp = ofToString(ofGetDay()) +
	ofToString(ofGetMonth()) +
	ofToString(ofGetYear()) +
	ofToString(ofGetHours()) +
	ofToString(ofGetMinutes()) +
	ofToString(ofGetSeconds());

	string _filename = (_root + _timestamp + ".oni");

	return _filename;

}
*/
//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

//	if (isCloud) pointCloudRotationY = x;

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

