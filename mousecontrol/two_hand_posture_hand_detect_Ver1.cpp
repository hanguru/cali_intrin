//============================================================================
// Name        : webcam_hand_posture.cpp
// Author      : Hong Ju Hyeon
// Version     :
// Copyright   : CAPP
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include "cv.h"
#include "highgui.h"
#include "ml.h"
#include <boost/program_options.hpp>
#include "ConvexityDefect.h"
#include "PostureClassifier.h"
using namespace std;
using namespace cv;

#define PI 3.1415926535

bool my_compare_function(double i, double j){ return (i>j); }

void draw_cross_line(Mat & img, Point center, int line_length, const Scalar& color, int thickness=1, int lineType=8, int shift=0 ){
	float x1 =  center.x - line_length/ (2 * sqrt(2 ));
	float y1 =  center.y - line_length/ (2 * sqrt(2 ));
	float x2 =  center.x + line_length/ (2 * sqrt(2 ));
	float y2 =  center.y + line_length/ (2 * sqrt(2 ));

	float x3 =  center.x + line_length/ (2 * sqrt(2 ));
	float y3 =  center.y - line_length/ (2 * sqrt(2 ));
	float x4 =  center.x - line_length/ (2 * sqrt(2 ));
	float y4 =  center.y + line_length/ (2 * sqrt(2 ));


	line(img, Point2f(x1,y1 ),Point2f(x2,y2), color, thickness, lineType, shift );
	line(img, Point2f(x3,y3 ),Point2f(x4,y4), color, thickness, lineType, shift );
}
bool parse_command_line(int argc, char *argv[], vector<string>&input_files, bool& debug, string& save_file){
	namespace po = boost::program_options;
	po::options_description desc("Usage: make_background [options] [<source files>] [<background image file>]\nAllowed options");

	desc.add_options()
					("help", "produce help message")
					("input,i", po::value<vector<string> >(), "list of input imagefiles")
					("debug,d", "show debug information")
					//("load,l", po::value<string>(&loadfilename), "filename to load")
					("save,s", po::value<string>(&save_file), "save result to filename")
					;

	po::options_description all;
	all.add(desc);

	po::positional_options_description p;
	p.add("input", -1);
	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).
				options(all).positional(p).run(), vm);
		po::notify(vm);
	} catch ( const boost::program_options::error& e ) {
		std::cerr << e.what() << std::endl;
		return false;
	}

	if(vm.count("input"))
	{
		input_files = vm["input"].as<vector<string> >();
		cout<<"Number of Input : "<<(int)input_files.size()<<endl;
	}

	if(vm.count("debug"))
	{
		debug = 1;
	}
	else
	{
		debug = 0;
	}
	if(vm.count("save"))
	{
		save_file = vm["save"].as<string>();
	}
	else
	{
		save_file = "";
	}
	return true;

}

int main(int argc, char* argv[]) {

	vector<string> input_file_list;
	bool debug = 0;
	bool posture_code;
	string save_file;
	VideoCapture video;
	Mat frame;
	Mat img;
	Mat rotated;
	PostureClassifier posture;

	if(!parse_command_line(argc,argv, input_file_list, debug, save_file))
	{
		return -1;
	}

	for(int i = 0 ; i < (int)input_file_list.size(); i++)
	{
		video.open(input_file_list[i]);
		if(!video.isOpened())  // check if we succeeded
				return -1;

		namedWindow("Camera",1);
		while(video.read(frame) != false)
		{
			img = frame.clone();
			//Mat trans = getRotationMatrix2D(Point2f(img.cols/2, img.rows/2),180 , 1);
			//	warpAffine(img.clone(), rotated, trans,img.clone().size());
			posture.open(img,Point(200,200));
			posture_code = posture.posture_detection();

			if(posture_code == 1)
			{
				cout<<" this is a palm"<<endl;
			}
			else
			{
				cout<<"this is a fist."<<endl;
			}


			imshow("input", img);
			imshow("Camera",posture.mask());
			waitKey(0);
		}
	}


	return 0;
}
