#include "msaFluidParticlesApp.h"

using namespace MSA;

char sz[] = "";


//--------------------------------------------------------------
void msaFluidParticlesApp::setup() {
	for(int i=0; i<strlen(sz); i++) sz[i] += 20;

	// setup fluid stuff
	fluidSolver.setup(100, 100);
    fluidSolver.enableRGB(true).setFadeSpeed(0.002).setDeltaT(0.5).setVisc(0.00015).setColorDiffusion(0);
	fluidDrawer.setup( &fluidSolver );
	particleSystem.setFluidSolver( &fluidSolver );

	fluidCellsX			= 150;

	drawFluid			= true;
	drawParticles		= true;

	ofSetFrameRate(60);
	ofBackground(0, 0, 0);
	ofSetVerticalSync(false);

	//Kinect
	kinect.init();
	//kinect.init(true);  // shows infrared instead of RGB video image
	//kinect.init(false, false);  // disable infrared/rgb video image (faster fps)
	kinect.setVerbose(true);
	kinect.open();
	// start with the live kinect source
	kinectSource = &kinect;
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	nearThreshold = 250;
	farThreshold  = 240;
	bThreshWithOpenCV = true;
// zero the tilt on startup
	angle = 3;
	kinect.setCameraTiltAngle(angle);


#ifdef USE_TUIO
	tuioClient.start(3333);
#endif


#ifdef USE_GUI
	gui.addSlider("fluidCellsX", fluidCellsX, 20, 400);
	gui.addButton("resizeFluid", resizeFluid);
	gui.addSlider("fs.viscocity", fluidSolver.viscocity, 0.0, 0.01);
	gui.addSlider("fs.colorDiffusion", fluidSolver.colorDiffusion, 0.0, 0.0003);
	gui.addSlider("fs.fadeSpeed", fluidSolver.fadeSpeed, 0.0, 0.1);
	gui.addSlider("fs.solverIterations", fluidSolver.solverIterations, 1, 50);
	gui.addSlider("fs.deltaT", fluidSolver.deltaT, 0.1, 5);
	gui.addComboBox("fd.drawMode", (int&)fluidDrawer.drawMode, kFluidDrawCount, (string*)FluidDrawerGl::drawOptionTitles);
	gui.addToggle("fs.doRGB", fluidSolver.doRGB);
	gui.addToggle("fs.doVorticityConfinement", fluidSolver.doVorticityConfinement);
	gui.addToggle("drawFluid", drawFluid);
	gui.addToggle("drawParticles", drawParticles);
	gui.addToggle("fs.wrapX", fluidSolver.wrap_x);
	gui.addToggle("fs.wrapY", fluidSolver.wrap_y);
	gui.setDefaultKeys(true);
	gui.currentPage().setXMLName("MSAFluidDemo.xml");
	gui.setAutoSave(false);
#endif

	windowResized(ofGetWidth(), ofGetHeight());		// force this at start (cos I don't think it is called)
	pMouse = getWindowCenter();
	resizeFluid			= true;

	ofEnableAlphaBlending();
	ofSetBackgroundAuto(true);
}


void msaFluidParticlesApp::fadeToColor(float r, float g, float b, float speed) {
    glColor4f(r, g, b, speed);
	ofRect(0, 0, ofGetWidth(), ofGetHeight());
}


// add force and dye to fluid, and create particles
void msaFluidParticlesApp::addToFluid( Vec2f pos, Vec2f vel, bool addColor, bool addForce ) {
    float speed = vel.x * vel.x  + vel.y * vel.y * getWindowAspectRatio() * getWindowAspectRatio();    // balance the x and y components of speed with the screen aspect ratio
    if(speed > 0) {
		pos.x = constrain(pos.x, 0.0f, 1.0f);
		pos.y = constrain(pos.y, 0.0f, 1.0f);

        const float colorMult = 100;
        const float velocityMult = 30;

        int index = fluidSolver.getIndexForPos(pos);

		if(addColor) {
			Color drawColor( CM_HSV, ( getElapsedFrames() % 360 ) / 360.0f, 1, 1 );
//			Color drawColor;
//			drawColor.setHSV(( getElapsedFrames() % 360 ) / 360.0f, 1, 1 );

			fluidSolver.addColorAtIndex(index, drawColor * colorMult);

			if( drawParticles )
				particleSystem.addParticles( pos * Vec2f( getWindowSize() ), 10 );
		}

		if(addForce)
			fluidSolver.addForceAtIndex(index, vel * velocityMult);

    }
}


void msaFluidParticlesApp::update(){
	if(resizeFluid) 	{
		fluidSolver.setSize(fluidCellsX, fluidCellsX / getWindowAspectRatio() );
		fluidDrawer.setup(&fluidSolver);
		resizeFluid = false;
	}

	//Kinect
	ofBackground(100, 100, 100);
	
	kinectSource->update();
	
	// there is a new frame and we are connected
	if(kinectSource->isFrameNew()) {
	
	// load grayscale depth image from the kinect source
	grayImage.setFromPixels(kinectSource->getDepthPixels(), kinect.width, kinect.height);
 	
	// we do two thresholds - one for the far plane and one for the near plane
	// we then do a cvAnd to get the pixels which are a union of the two thresholds 
	if(bThreshWithOpenCV) {
		grayThreshNear = grayImage;
		grayThreshFar = grayImage;	
		grayThreshNear.threshold(nearThreshold, true);
		grayThreshFar.threshold(farThreshold);
		cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
	}

	// update the cv images
	grayImage.flagImageChanged();
	
	// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);

}

#ifdef USE_TUIO
	tuioClient.getMessage();

	// do finger stuff
	list<ofxTuioCursor*>cursorList = tuioClient.getTuioCursors();
	for(list<ofxTuioCursor*>::iterator it=cursorList.begin(); it != cursorList.end(); it++) {
		ofxTuioCursor *tcur = (*it);
        float vx = tcur->getXSpeed() * tuioCursorSpeedMult;
        float vy = tcur->getYSpeed() * tuioCursorSpeedMult;
        if(vx == 0 && vy == 0) {
            vx = ofRandom(-tuioStationaryForce, tuioStationaryForce);
            vy = ofRandom(-tuioStationaryForce, tuioStationaryForce);
        }

        Vec2f tuiopos = Vec2f(tcur->getX(), tcur->getY());
        Vec2f tuiovel = Vec2f(vx, vy);

        addToFluid(tuiopos, tuiovel, true, true);
    }
#endif

// OpenCV + Kinect as MultiTouch
	
	float kinectwinfactorX = 1.7; // ofGetWidth() / 1024;
	float kinectwinfactorY = 1.5; // ofGetHeight() / 768;

	if (kinect.isConnected()) {
		for (int i = 0; i < contourFinder.nBlobs; i++){  
			contourFinder.blobs[i].draw(360,20);  
  
			float getX = contourFinder.blobs.at(i).centroid.x * kinectwinfactorX;  
			float getY = contourFinder.blobs.at(i).centroid.y * kinectwinfactorY;  
//			Vec2f kinectPos = Vec2f(ofGetWidth() - getX, getY);
			Vec2f kinectPos = Vec2f(getX, getY);
			Vec2f kinectNorm = Vec2f( kinectPos) / getWindowSize();
			float stationaryForce = 0.01f;

			Vec2f kinectVel = Vec2f(ofRandom(-stationaryForce, stationaryForce), ofRandom(-stationaryForce, stationaryForce));
			addToFluid(kinectNorm, kinectVel, true, true);
		}
    }




	fluidSolver.update();
}

void msaFluidParticlesApp::draw(){
    
	if( drawFluid ) {
		glColor3f(1, 1, 1);
		fluidDrawer.draw(0, 0, getWindowWidth(), getWindowHeight());
	} else {
		if(getElapsedFrames()%5==0) fadeToColor( 0, 0, 0, 0.1f );
	}
	if( drawParticles )
		particleSystem.updateAndDraw( drawFluid );

	ofDrawBitmapString(sz, 50, 50);

#ifdef USE_GUI
	gui.draw();
#endif

	//Kinect
	ofSetColor(255, 255, 255);
	
	// draw from the live kinect
	// kinect.drawDepth(10, 10, 400, 300);
	// kinect.draw(420, 10, 400, 300);
	// grayImage.draw(10, 320, 400, 300);
	// contourFinder.draw(10, 320, 400, 300);
	if(gui.isOn()) {
	grayImage.draw(ofGetWidth()-kinect.width/2-10, ofGetHeight()-kinect.height/2-10-100, kinect.width/2, kinect.height/2); //Position: lower right corner
	contourFinder.draw(ofGetWidth()-kinect.width/2-10, ofGetHeight()-kinect.height/2-10-100, kinect.width/2, kinect.height/2); //Position: lower right corner

	// draw instructions
	ofSetColor(200, 200, 200);
	stringstream reportStream;
	reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
								 << ofToString(kinect.getMksAccel().y, 2) << " / " 
								 << ofToString(kinect.getMksAccel().z, 2) << endl
				 << "set near threshold " << nearThreshold << " (press: + -)" << endl
				 << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
				 	<< ", fps: " << ofGetFrameRate() << endl
				 << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl
				 << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl;
	ofDrawBitmapString(reportStream.str(),ofGetWidth()-600,ofGetHeight()-80);
	}
}

//--------------------------------------------------------------
void msaFluidParticlesApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

void msaFluidParticlesApp::windowResized(int w, int h) {
	particleSystem.setWindowSize( Vec2f( w, h ) );
}


void msaFluidParticlesApp::keyPressed  (int key){
    switch(key) {
		case '1':
			fluidDrawer.setDrawMode(MSA::kFluidDrawColor);
			break;

		case '2':
			fluidDrawer.setDrawMode(MSA::kFluidDrawMotion);
			break;

		case '3':
			fluidDrawer.setDrawMode(MSA::kFluidDrawSpeed);
			break;

		case '4':
			fluidDrawer.setDrawMode(MSA::kFluidDrawVectors);
			break;

		case 'd':
			drawFluid ^= true;
			break;

		case 'p':
			drawParticles ^= true;
			break;

		case 'f':
			ofToggleFullscreen();
			break;

		case 'r':
			fluidSolver.reset();
			break;

		case 'b': {
			Timer timer;
			const int ITERS = 3000;
			timer.start();
			for( int i = 0; i < ITERS; ++i ) fluidSolver.update();
			timer.stop();
			cout << ITERS << " iterations took " << timer.getSeconds() << " seconds." << std::endl;
		}
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
void msaFluidParticlesApp::mouseMoved(int x, int y ){
	Vec2f eventPos = Vec2f(x, y);
	Vec2f mouseNorm = Vec2f( eventPos) / getWindowSize();
	Vec2f mouseVel = Vec2f( eventPos - pMouse ) / getWindowSize();
	addToFluid( mouseNorm, mouseVel, true, true );
	pMouse = eventPos;
}

void msaFluidParticlesApp::mouseDragged(int x, int y, int button) {
	Vec2f eventPos = Vec2f(x, y);
	Vec2f mouseNorm = Vec2f( eventPos ) / getWindowSize();
	Vec2f mouseVel = Vec2f( eventPos - pMouse ) / getWindowSize();
	addToFluid( mouseNorm, mouseVel, false, true );
	pMouse = eventPos;
}

