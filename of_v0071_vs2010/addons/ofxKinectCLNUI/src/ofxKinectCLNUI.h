/*
*  ofxKinectCLNUI.h
*
*  Created by yangyang on 8/1/2011.
*  Welcome to visit http://www.cnblogs.com/yangyangcv/
*
*  CL NUI SDK is developed by AlexP
*  see http://codelaboratories.com/nui/
*/

#ifndef OFXKINECTCLNUI_H_INCLUDED
#define OFXKINECTCLNUI_H_INCLUDED

#include "CLNUIDevice.h"
#include "cv.h" 

class ofxKinectCLNUI
{
private:
	CLNUICamera kinectCam;
	int camWidth;
	int camHeight;
	IplImage *depthShortImage;
	IplImage *bgrImage;
	
	USHORT *depthData;
	BYTE *colorData;
	
public:
	ofxKinectCLNUI();
	~ofxKinectCLNUI();
	bool initKinect(int wid,int hei,double lowbound,double upbound);
	void setUpBound(double bound);//up and low threshold for the depth data
	void setLowBound(double bound);
	bool getDepthBW();//threshold on the depth date we can get this BW image
	bool getColorImage();//color image from the rgb camera
	int getCamWidth();
	int getCamHeight();

	double upBound, lowBound;//up and low threshold for the depth data
	IplImage *bwImage;//threshold on the depth date we can get this BW image
	IplImage *rgbImage;//image from the rgb camera
};

#endif