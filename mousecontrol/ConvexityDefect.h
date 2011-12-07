/*
 * ConvexityDefect.h
 *
 *  Created on: 2011. 8. 4.
 *      Author: Hong Ju Hyeon
 */

#ifndef CONVEXITYDEFECT_H_
#define CONVEXITYDEFECT_H_

#include <cv.h>
#include <highgui.h>
#include <vector>
class ConvexityDefect
{
public:
	cv::Point start;
	cv::Point end;
	cv::Point depth_point;
	float depth;
};

void findConvexityDefects(vector<cv::Point>& contour, vector<int>& hull, vector<ConvexityDefect>& convexDefects);
void findConvexityDefects(vector<cv::Point>& contour, vector<ConvexityDefect>& convexDefects);


#endif /*CONVEXITYDEFECT_H_*/


