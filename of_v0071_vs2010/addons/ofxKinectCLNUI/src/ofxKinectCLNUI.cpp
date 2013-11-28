#include "ofxKinectCLNUI.h"
#include <stdio.h>
ofxKinectCLNUI::ofxKinectCLNUI()
{
	
}

bool ofxKinectCLNUI::initKinect(int wid,int hei,double lowbound,double upbound)
{
	bool b = false;
	upBound = upbound;
	lowBound = lowbound;

	camWidth = wid;
	camHeight = hei;
	kinectCam= CreateNUICamera();
	b = StartNUICamera(kinectCam);
	//depth image
	depthShortImage = cvCreateImage(cvSize(camWidth,camHeight),IPL_DEPTH_16U,1);
	//BGR image
	bgrImage = cvCreateImage(cvSize(camWidth,camHeight),IPL_DEPTH_8U,3);
	//RGB image
	rgbImage = cvCreateImage(cvSize(camWidth,camHeight),IPL_DEPTH_8U,3);
	//BW image
	bwImage = cvCreateImage(cvSize(camWidth,camHeight),IPL_DEPTH_8U,1);

	depthData = new USHORT[camWidth*camHeight];
	colorData = new BYTE[camWidth*camHeight*3];

	return b;
}

void ofxKinectCLNUI::setLowBound(double bound)
{
	lowBound = bound;
}

void ofxKinectCLNUI::setUpBound(double bound)
{
	upBound = bound;
}

bool ofxKinectCLNUI::getDepthBW()
{
	bool b = GetNUICameraDepthFrameRAW(kinectCam, depthData);//gray scale depth image

	for( int y=0; y<camHeight; y++ ) {
		USHORT* ptr = (USHORT*) (depthShortImage->imageData + y * depthShortImage->widthStep	);

		for( int x=0; x<camWidth; x++ ) {
			ptr[x] = depthData[y*camWidth+x];
		}
	}
	//we use cvInRangeS() to do the thresholding
	cvInRangeS( depthShortImage, cvScalar(lowBound), cvScalar(upBound), bwImage );

	return b;
}

bool ofxKinectCLNUI::getColorImage()
{
	bool b = GetNUICameraColorFrameRGB24(kinectCam, colorData);
	bgrImage->imageData = (char*)colorData;
	cvCvtColor(bgrImage,rgbImage,CV_BGR2RGB);
	return b;
}
int ofxKinectCLNUI::getCamWidth()
{
	return camWidth;
}

int ofxKinectCLNUI::getCamHeight()
{
	return camHeight;
}

ofxKinectCLNUI::~ofxKinectCLNUI()
{
	StopNUICamera(kinectCam);
	DestroyNUICamera(kinectCam);

	cvReleaseImage(&bgrImage);
	cvReleaseImage(&rgbImage);
	cvReleaseImage(&depthShortImage);
	cvReleaseImage(&bwImage);

	delete depthData;
	delete colorData;
}