/*
 * PostureClassifier.cpp
 *
 *  Created on: 2011. 10. 27.
 *      Author: JuHyeon Hong
 */

#include "stdafx.h"
//#define _Debug 0;

#include "PostureClassifier.h"
using namespace cv;
using namespace std;

PostureClassifier::PostureClassifier(){
}
PostureClassifier::PostureClassifier(const cv::Mat & source_img){
	open(source_img);
}
PostureClassifier::PostureClassifier(const cv::Mat & source_img, cv::Point lowest_point){
	open(source_img);
	m_lowest_value = lowest_point;
}
PostureClassifier::PostureClassifier(const cv::Mat & source_img, cv::Point lowest_point, int threshold){
	open(source_img);
	m_lowest_value = lowest_point;
	m_threshold = threshold;
}
PostureClassifier::PostureClassifier(const cv::Mat &source_img, unsigned int area){
	open(source_img);
	m_area = area;
}
PostureClassifier::~PostureClassifier() {
}

bool PostureClassifier::open(const cv::Mat & source_img){

	//initialize
	m_histogram_depth = vector<float>(0,0);
	//m_histogram_depth = vector<float>(12,0);
	m_defect_number_in_histogram = vector<int>(12,-1);
	m_lowest_value = Point(-1,-1);
	m_input_img = source_img;
	m_threshold = 200;
	m_area = 0;
	m_roi_tl = Point (0,0);
	return true;

}

bool PostureClassifier::open(const cv::Mat & source_img, cv::Point lowest_point){
	open(source_img);
	m_lowest_value = lowest_point;
	return true;
}
bool PostureClassifier::open(const cv::Mat & source_img, cv::Point lowest_point, int threshold){
	open(source_img);
	m_lowest_value = lowest_point;
	m_threshold = threshold;
	return true;
}

void PostureClassifier::threshold_cut()
{

	int tl_x;
	int tl_y;
	int br_x;
	int br_y;
	if(m_lowest_value.x - 100 < 0)
	{
		tl_x = 0;
	}
	else
	{
		tl_x = m_lowest_value.x - 100;
	}

	if(m_lowest_value.x + 100 > m_input_img.cols)
	{
		br_x = m_input_img.cols;
	}
	else
	{
		br_x = m_lowest_value.x + 100;
	}

	if(m_lowest_value.y - 100 < 0)
	{
		tl_y = 0;
	}
	else
	{
		tl_y = m_lowest_value.y - 100;
	}

	if(m_lowest_value.y + 100 > m_input_img.rows)
	{
		br_y = m_input_img.rows;
	}
	else
	{
		br_y = m_lowest_value.y + 100;
	}

	m_roi_tl = Point (tl_x,tl_y);
	Mat roi(m_input_img, Rect(Point(tl_x, tl_y),Point(br_x, br_y)));
	Mat temp(roi.rows,roi.cols, CV_8UC1);
	m_mask = temp.clone();
	m_mask.setTo(Scalar(0));

	int count_pixel = 0;
	int sum_of_pixel = 0;

	for (int i = 0 ; i < (int)roi.rows; i++)
	{
		for(int j = 0 ; j < (int)roi.cols; j++)
		{
			if (roi.at<unsigned short>(i,j) > 30)
			{
				if( roi.at<unsigned short>(i,j) >= m_input_img.at<unsigned short>(m_lowest_value.y, m_lowest_value.x) &&
						roi.at<unsigned short>(i,j) < m_input_img.at<unsigned short>(m_lowest_value.y, m_lowest_value.x) + m_threshold )
				{
					sum_of_pixel = sum_of_pixel +  m_input_img.at<unsigned short>(m_lowest_value.y, m_lowest_value.x);
					count_pixel ++;
					m_area++;
					m_mask.at<unsigned char>(i,j) = 255;
				}
			}
		}
	}
	if( count_pixel != 0 )
		m_hand_distance = sum_of_pixel / count_pixel;
	else
	{
		m_hand_distance = 65535;
	}

}


void PostureClassifier::extract_contour(){
	m_largest_contour = vector<Point>(0,Point(0,0));
	vector<vector<Point> >contour;
	Mat temp = m_mask.clone();
	int contour_area = 0;

	findContours(temp, contour, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for(int i = 0; i< (int)contour.size(); i++)
	{
		if(contour_area < contourArea(Mat(contour[i])))
		{
			contour_area = contourArea(Mat(contour[i]));
			m_largest_contour = contour[i];
		}
	}

}
void PostureClassifier::extract_convexHull(){
	m_hull = vector<Point>(0,Point(0,0));
	convexHull(Mat(m_largest_contour), m_hull, CV_CLOCKWISE);
}


void PostureClassifier::extract_defect(){
	m_defect = vector<ConvexityDefect>(0);
	findConvexityDefects(m_largest_contour, m_defect);

}

int PostureClassifier::find_highest_rows(){
	for(int i = 0 ; i < m_mask.rows; i++)
	{
		for(int j = 0; j < m_mask.cols; j++)
		{
			if(m_mask.at<unsigned char>(i,j) != 0)
			{
				return i;
			}

		}
	}
	return 0;
}


void PostureClassifier::cut_image(){

	Mat mask(0,m_mask.cols,CV_8UC1);
	Mat zero_mask(1,m_mask.cols,CV_8UC1);
	zero_mask.setTo(Scalar(0));
	int row;

	row = find_highest_rows();

	for (int i = 0; i < m_mask.rows; i++)
	{
		if(i < row + hand_size_diameter()*2)
		{
			mask.push_back(m_mask.row(i));
		}
		else
		{
			mask.push_back(zero_mask);
		}
	}
	m_mask = mask;
}

void PostureClassifier::rotate_alignment( Point low_point){

	low_point = Point(m_mask.cols/2, m_mask.rows/2);
	Vec4f line;
	fitLine(Mat(m_hull),line,CV_DIST_L2,0,0.01,0.01);
	m_center_of_image = Point(line.val[2],line.val[3]);



#ifdef _Debug
	Mat test;
	cvtColor(m_mask,test,CV_GRAY2BGR);
	circle(test,low_point,5,Scalar(0,255,0));
	circle(test,Point(line.val[2],line.val[3]),5,Scalar(0,0,255));
	imshow("test",test);
#endif
	Mat img;
	int angle;

	if( (line.val[2] < low_point.x && line.val[3] == low_point.y ) || line.val[3] > low_point.y ){
		if(atan2(line.val[0],-1*line.val[1])*180/3.141592 > 90)
		{

			angle = atan2(line.val[0],-1*line.val[1])*180/3.141592 - 180;
			//cout<<atan2(line.val[0],-1*line.val[1])*180/3.141592<<endl;
			//cout<<"case1"<<endl;
		}
		else
		{
			angle = atan2(line.val[0],-1*line.val[1])*180/3.141592;
			//cout<<atan2(line.val[0],-1*line.val[1])*180/3.141592<<endl;
			//cout<<"case2"<<endl;
		}
	}
	else if ((line.val[2] > low_point.x && line.val[3] == low_point.y) || line.val[3] < low_point.y)
	{
		if(atan2(line.val[0],-1*line.val[1])*180/3.141592 > 90)
		{
			//cout<<"3"<<endl;
			angle = atan2(line.val[0],-1*line.val[1])*180/3.141592;
			//cout<<atan2(line.val[0],-1*line.val[1])*180/3.141592<<endl;
			//cout<<"case3"<<endl;
		}
		else
		{
			//cout<<"4"<<endl;
			angle = atan2(line.val[0],-1*line.val[1])*180/3.141592+180;
			//cout<<atan2(line.val[0],-1*line.val[1])*180/3.141592<<endl;
			//cout<<"case4"<<endl;
		}
	}


	Mat trans = getRotationMatrix2D(Point2f(m_mask.cols/2, m_mask.rows/2),angle , 1);

	warpAffine(m_mask.clone(), img, trans,m_mask.clone().size());



	m_mask = img;



}

void PostureClassifier::make_histogram()
{

	for(int i = 0 ; i < (int)m_defect.size(); i++)
	{
		m_histogram_depth.push_back(m_defect[i].depth/hand_size_diameter());
	}


	//below lines are old article version.
//	float x;
//	float y;
//	float angle;
//	m_total_sum_histogram = 0;
//
//
//	for(int i = 0 ; i < (int)m_defect.size(); i++)
//	{
//		x =	( m_defect[i].depth_point.x + m_defect[i].start.x + m_defect[i].end.x) / 3;
//	    y = ( m_defect[i].depth_point.y + m_defect[i].start.y + m_defect[i].end.y) / 3;
//	    if( angle < 0)
//	    	angle = atan2(m_center_of_hand.y - y, x  - m_center_of_hand.x)*180/3.141592 + 360;
//	    else
//	    	angle = atan2(m_center_of_hand.y - y, x  - m_center_of_hand.x)*180/3.141592;
//
//	    if( angle >= 0 && angle < 30  )
//	    {
//	    	if(m_histogram_depth[0] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[0] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[0] = i;
//	    	}
//	    }
//
//	    else if( angle >= 30 && angle < 60  )
//	    {
//	    	if(m_histogram_depth[1] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[1] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[1] = i;
//	    	}
//	    }
//
//	    else if( angle >= 60 && angle < 90  )
//	    {
//	    	if(m_histogram_depth[2] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[2] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[2] = i;
//	    	}
//	    }
//
//	    else if( angle >= 90 && angle < 120  )
//	    {
//	    	if(m_histogram_depth[3] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[3] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[3] = i;
//	    	}
//	    }
//
//	    else if( angle >= 120 && angle < 150  )
//	    {
//	    	if(m_histogram_depth[4] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[4] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[4] = i;
//	    	}
//	    }
//
//	    else if( angle >= 150 && angle < 180  )
//	    {
//	    	if(m_histogram_depth[5] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[5] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[5] = i;
//	    	}
//	    }
//
//	    else if( angle >= 180 && angle < 210  )
//	    {
//	    	if(m_histogram_depth[6] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[6] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[6] = i;
//	    	}
//	    }
//
//	    else if( angle >= 210 && angle < 240  )
//	    {
//	    	if(m_histogram_depth[7] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[7] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[7] = i;
//	    	}
//	    }
//
//	    else if( angle >= 240 && angle < 270  )
//	    {
//	    	if(m_histogram_depth[8] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[8] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[8] = i;
//	    	}
//	    }
//
//	    else if( angle >= 270 && angle < 300  )
//	    {
//	    	if(m_histogram_depth[9] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[9] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[9] = i;
//	    	}
//	    }
//
//	    else if( angle >= 300 && angle < 330  )
//	    {
//	    	if(m_histogram_depth[10] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[10] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[10] = i;
//	    	}
//	    }
//
//	    else if( angle >= 330 && angle < 360  )
//	    {
//	    	if(m_histogram_depth[11] < m_defect[i].depth)
//	    	{
//	    		m_histogram_depth[11] = m_defect[i].depth;
//	    		m_defect_number_in_histogram[11] = i;
//	    	}
//	    }
//	}
//
//	//below for loop makes scale invariant feature
//	for (int i = 0 ; i < (int)m_histogram_depth.size(); i++)
//	{
//		m_histogram_depth[i] = m_histogram_depth[i] / (float)hand_size_diameter();
//	}
//#ifdef _Debug
//	cout<<"denormalized: "<< Mat(m_histogram_depth)<<endl;
//#endif

	//below 2 for loop means normalization
//	for (int i = 0 ; i < (int)m_histogram_depth.size(); i++)
//	{
//		m_total_sum_histogram = m_total_sum_histogram + m_histogram_depth[i];
//	}
//
//	for (int i = 0 ; i < (int)m_histogram_depth.size(); i++)
//	{
//		m_histogram_depth[i] = m_histogram_depth[i] / m_total_sum_histogram;
//	}
//
//	cout<<"normalized :" <<Mat(m_histogram_depth)<<endl;

}

int PostureClassifier::classify_posture()
{
	std::sort(m_histogram_depth.begin(), m_histogram_depth.end(),greater<float>());
#ifdef _Debug
	vector<float> debug;
	debug.assign(m_histogram_depth.begin(), m_histogram_depth.begin()+7 );
	cout<<Mat(debug)<<endl;
#endif
	if((int)m_histogram_depth.size()>3)
	{

		if(m_histogram_depth[2] > 0.3)
			return 1;

		else
			return 0;
	}
	else{
		return 0;
	}
}



//	int number_of_major_defect = 0;
//
//	float maximum_value_histogram;
//	maximum_value_histogram = *max_element(m_histogram_depth.begin(),m_histogram_depth.end());
//	//cout<<maximum_value_histogram<<endl;
//	vector<int> major_defect_index;
//
//	for(int i = 0 ; i < (int)m_histogram_depth.size(); i++)
//	{
//		if( m_histogram_depth[i] > 0.2 )
//		{
//			number_of_major_defect ++;
//			major_defect_index.push_back(i);
//		}
//		//m_histogram_depth[i] = m_histogram_depth[i] * m_total_sum_histogram;
//	}
//	std::sort(m_histogram_depth.begin(), m_histogram_depth.end(),greater<float>());
//
//	//cout<<"determine: "<<Mat(m_histogram_depth)<<endl;
//#ifdef _Debug
//	cout<<number_of_major_defect<<endl;
//#endif
//	if(number_of_major_defect > 2)
//	{
//		if(m_histogram_depth[0] > 0.18 && m_histogram_depth[1] > 0.18)
//		{
//			//cout<<"defect number >2 & hand"<<endl;
//			return 1;
//		}
//		else
//		{
//			//cout<<"defect number >2 & fist"<<endl;
//			return 0;
//		}
//	}
//
//	else if (number_of_major_defect == 2 )
//	{
//		if(m_histogram_depth[0] > 0.3)
//		{
//			return 1;
//		}
//		else
//		{
//			return check_distribution(major_defect_index);//cout<<"defect number = 2 & hand or fist"<<endl;
//		}
//
//	}
//
//	else if (number_of_major_defect == 1)
//	{
//		if(m_histogram_depth[0] > 0.3)
//		{
//			//cout<<"defect number = 1  & hand"<<endl;
//			//cout<<m_histogram_depth[0]<<endl;
//			return 1;
//		}
//		else
//		{
//			//cout<<"defect number  =1  & fist"<<endl;
//			return 0;
//		}
//	}
//	else
//	{
//		//cout<<"defect number = 0, so fist"<<endl;
//		return 0;
//	}
//return -1;
//}

bool PostureClassifier::check_distribution(const vector<int>& index)
{
#ifdef _Debug
	if( index.size() != 2)
	{
		cout<<"Error!!!"<<endl;
	}
#endif

	float d1;
	float d2;
	float d3;

	d1 = sqrt
			((float)
					(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].end.x)
					+
					(m_defect[m_defect_number_in_histogram[index[0]]].start.y - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].end.y)
			);

	d2 = sqrt
			((float)
					(m_defect[m_defect_number_in_histogram[index[0]]].end.x - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].start.x)
					+
					(m_defect[m_defect_number_in_histogram[index[0]]].end.y - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].start.y)
			);

	d3 = sqrt
			((float)
					(m_defect[m_defect_number_in_histogram[index[0]]].depth_point.x - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].depth_point.x)
					+
					(m_defect[m_defect_number_in_histogram[index[0]]].depth_point.y - m_defect[m_defect_number_in_histogram[index[1]]].end.x) *(m_defect[m_defect_number_in_histogram[index[0]]].start.x - m_defect[m_defect_number_in_histogram[index[1]]].depth_point.y)
			);

	if (d3 < d1 && d3 < d2)
	{
		return 0;
	}

	else
	{
		return 1;
	}
//#ifdef _Debug
//	else
//	{
//		cout<<"error!"<<endl;
//		return 0;
//	}
//#endif
}

void PostureClassifier::set_center_of_hand(){
	int x_max = numeric_limits<int>::min();
	int x_min = numeric_limits<int>::max();
	int y_max = numeric_limits<int>::min();
	int y_min = numeric_limits<int>::max();


	for(int i = 0 ; i < (int)m_hull.size(); i++)
	{
		if(m_hull[i].x > x_max)
		{
			x_max = m_hull[i].x;
		}
		if(m_hull[i].x < x_min)
		{
			x_min = m_hull[i].x;
		}

		if(m_hull[i].y > y_max)
		{
			y_max = m_hull[i].y;
		}
		if(m_hull[i].y < y_min)
		{
			y_min = m_hull[i].y;
		}
	}

	m_center_of_hand = Point( (x_max + x_min)/ 2 , (y_max + y_min) /2);
}

int PostureClassifier::posture_detection()
{
	if(m_input_img.total() == 0 && m_mask.total() == 0)
	{
		m_center_of_hand = m_lowest_value;
		return -1;
	}

	threshold_cut();

	if( hand_size_diameter() < 25)
	{
		m_center_of_hand = m_lowest_value;
		return -1;
	}

	if ( m_area < 1000)
	{
		m_center_of_hand = m_lowest_value;
		return -1;
	}
	else
	{

		extract_contour();
		extract_convexHull();

		extract_defect();

		rotate_alignment(m_lowest_value);

		//cv::line(m_mask, Point2f(line.val[2],line.val[3]), Point2f(line.val[2] - 100*(-1 * line.val[0] / line.val[1]),line.val[3]+100),Scalar(125),2);
		cut_image();

		extract_contour();
		extract_convexHull();
		set_center_of_hand();
		extract_defect();
		make_histogram();
#ifdef _Debug
//		double convexity_defect_area = 0;
		vector<vector<Point> > defect_list;
		//cout<<Mat(m_defect_number_in_histogram)<<endl;
		for (int i  = 0 ; i < (int)m_defect_number_in_histogram.size(); i++)
		{
			if(m_defect_number_in_histogram[i] != -1)
			{
			vector<Point> defects;

			defects.push_back(m_defect[m_defect_number_in_histogram[i]].start);
			defects.push_back(m_defect[m_defect_number_in_histogram[i]].depth_point);
			defects.push_back(m_defect[m_defect_number_in_histogram[i]].end);
			defect_list.push_back(defects);
//			convexity_defect_area = convexity_defect_area + contourArea(Mat(defects));
			}
		}
		Mat camera_3ch;
		cvtColor(m_mask,camera_3ch,CV_GRAY2BGR);

		vector<vector<Point> >convex;
		convex.push_back(m_hull);

		drawContours(camera_3ch,defect_list,-1,Scalar(0,0,255),2);
		drawContours(camera_3ch,convex,-1,Scalar(0,255,0),1);
		circle(camera_3ch,m_center_of_hand,2,Scalar(0,255,0),3);
		imshow("debug_Defect",camera_3ch);

//
//		cout<<convexity_defect_area/ contourArea(Mat(m_hull))<<endl;
//
//		if ( convexity_defect_area/ contourArea(Mat(m_hull)) > 0.15 )
//		{
//			return 1;
//		}
//		else{
//			return 0;
//		}


#endif

		return classify_posture();
	}
}
