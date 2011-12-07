/*
 * PostureClassifier.h
 *
 *  Created on: 2011. 10. 27.
 *      Author: Hong Ju Hyeon
 */

#ifndef POSTURECLASSIFIER_H_
#define POSTURECLASSIFIER_H_
#include <algorithm>
#include <vector>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <cmath>
#include "ConvexityDefect.h"


class PostureClassifier {
public:
	PostureClassifier();
	PostureClassifier(const cv::Mat &source_img);
	PostureClassifier(const cv::Mat &source_img, cv::Point lowest_point);
	PostureClassifier(const cv::Mat &source_img, cv::Point lowest_point, int threshold);
	PostureClassifier(const cv::Mat &source_img, unsigned int area);
	virtual ~PostureClassifier();


	//accessor
	bool open(const cv::Mat & source_img);
	bool open(const cv::Mat & source_img, cv::Point lowest_point);
	bool open(const cv::Mat & source_img, cv::Point lowest_point, int threshold);
	const cv::Mat &mask() const { return m_mask; }
	const std::vector<cv::Point> &hull() const { return m_hull; }
	const std::vector<ConvexityDefect> &defect() const { return m_defect; }
	cv::Mat &mask() { return m_mask; }
	cv::Point center_of_hand(){return cv::Point(m_center_of_hand.x+m_roi_tl.x,m_center_of_hand.y+m_roi_tl.y) ;}
	std::vector<cv::Point> &hull() { return m_hull; }
	std::vector<ConvexityDefect> &defect() { return m_defect; }



	//image operation
	void threshold_cut();
	void extract_contour();
	void extract_convexHull();
	void extract_defect();
	int hand_size_diameter(){return (m_hand_distance - 1028) * (-1*19)/313 + 60 ;}

	//data extraction
	void set_center_of_hand();
	void make_histogram();


	//whole process
	int posture_detection();

	//classifying
	int classify_posture();

private:

	int find_highest_rows();
	void rotate_alignment(cv::Point low_point);
	void cut_image();
	bool check_distribution(const vector<int>& index);

	cv::Point m_lowest_value;

	cv::Point m_center_of_image;
	cv::Point m_center_of_hand;
	cv::Point m_roi_tl;
	cv::Mat m_input_img;
	cv::Mat m_mask;
	//std::vector<std::vector<cv::Point> >m_contour;
	std::vector<cv::Point> m_largest_contour;
	std::vector<cv::Point> m_hull;
	std::vector<ConvexityDefect> m_defect;


	std::vector<float> m_histogram_depth;
	std::vector<int> m_defect_number_in_histogram;
	//std::tr1::shared_ptr<vector<int> >m_defect_number_in_histogram_ptr;
	float m_maximum_depth;
	unsigned int m_area;
	float m_total_sum_histogram;

	int m_hand_distance;
	int m_threshold;

};

#endif /* POSTURECLASSIFIER_H_ */
