/*
 * ConvexityDefect.cpp
 *
 *  Created on: 2011. 10. 27.
 *      Author: Administrator
 */
#include "stdafx.h"
#include "ConvexityDefect.h"

void findConvexityDefects(vector<cv::Point>& contour, vector<int>& hull, vector<ConvexityDefect>& convexDefects){
	if(hull.size() > 0 && contour.size() > 0)
	{    CvSeq* contourPoints;
	CvSeq* defects;
	CvMemStorage* storage;
	CvMemStorage* strDefects;
	CvMemStorage* contourStr;
	CvConvexityDefect *defectArray = 0;
	strDefects = cvCreateMemStorage();
	defects = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvSeq),sizeof(CvPoint), strDefects );
	//We transform our vector<cv::Point> into a CvSeq* object of CvPoint.
	contourStr = cvCreateMemStorage();
	contourPoints = cvCreateSeq(CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), contourStr);
	for(int i=0; i<(int)contour.size(); i++)
	{
		CvPoint cp = {contour[i].x,  contour[i].y};
		cvSeqPush(contourPoints, &cp);
	}
	//Now, we do the same thing with the hull index
	int count = (int)hull.size();   //int hullK[count];
	int* hullK = (int*)malloc(count*sizeof(int));
	for(int i=0; i<count; i++)
	{
		hullK[i] = hull.at(i);
	}
	CvMat hullMat = cvMat(1, count, CV_32SC1, hullK);
	//We calculate convexity defects
	storage = cvCreateMemStorage(0);
	defects = cvConvexityDefects(contourPoints, &hullMat, storage);
	defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*defects->total);
	cvCvtSeqToArray(defects, defectArray, CV_WHOLE_SEQ);
	//printf("DefectArray %i %i\n",defectArray->end->x, defectArray->end->y);
	//We store defects points in the convexDefects parameter.
	for(int i = 0; i<defects->total; i++)
	{
		ConvexityDefect c;
		c.start.x = defectArray[i].start->x;
		c.start.y = defectArray[i].start->y;
		c.end.x = defectArray[i].end->x;
		c.end.y = defectArray[i].end->y;
		c.depth_point.x = defectArray[i].depth_point->x;
		c.depth_point.y = defectArray[i].depth_point->y;
		c.depth = defectArray[i].depth;
		convexDefects.push_back(c);
	}
	//We release memory (I think I forgot to free something hehe)
	cvReleaseMemStorage(&contourStr);
	cvReleaseMemStorage(&strDefects);
	cvReleaseMemStorage(&storage);
	}
}

void findConvexityDefects(vector<cv::Point>& contour, vector<ConvexityDefect>& convexDefects){
	if( contour.size() > 0)
	{    CvSeq* contourPoints;
	CvSeq* defects;
	CvMemStorage* storage;
	CvMemStorage* strDefects;
	CvMemStorage* contourStr;
	CvConvexityDefect *defectArray = 0;
	strDefects = cvCreateMemStorage();
	defects = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvSeq),sizeof(CvPoint), strDefects );
	//We transform our vector<cv::Point> into a CvSeq* object of CvPoint.
	contourStr = cvCreateMemStorage();
	contourPoints = cvCreateSeq(CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), contourStr);
	for(int i=0; i<(int)contour.size(); i++)
	{
		CvPoint cp = {contour[i].x,  contour[i].y};
		cvSeqPush(contourPoints, &cp);
	}
	//Now, we do the same thing with the hull index
	vector<int> hull;
	convexHull((cv::Mat)contour,hull,CV_CLOCKWISE);
	int count = (int)hull.size();   //int hullK[count];
	int* hullK = (int*)malloc(count*sizeof(int));
	for(int i=0; i<count; i++)
	{
		hullK[i] = hull.at(i);
	}
	CvMat hullMat = cvMat(1, count, CV_32SC1, hullK);
	//We calculate convexity defects
	storage = cvCreateMemStorage(0);
	defects = cvConvexityDefects(contourPoints, &hullMat, storage);
	defectArray = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*defects->total);
	cvCvtSeqToArray(defects, defectArray, CV_WHOLE_SEQ);
	//printf("DefectArray %i %i\n",defectArray->end->x, defectArray->end->y);
	//We store defects points in the convexDefects parameter.
	for(int i = 0; i<defects->total; i++)
	{
		ConvexityDefect c;
		c.start.x = defectArray[i].start->x;
		c.start.y = defectArray[i].start->y;
		c.end.x = defectArray[i].end->x;
		c.end.y = defectArray[i].end->y;
		c.depth_point.x = defectArray[i].depth_point->x;
		c.depth_point.y = defectArray[i].depth_point->y;
		c.depth = defectArray[i].depth;
		convexDefects.push_back(c);
	}
	//We release memory (I think I forgot to free something hehe)
	cvReleaseMemStorage(&contourStr);
	cvReleaseMemStorage(&strDefects);
	cvReleaseMemStorage(&storage);
	}
}
