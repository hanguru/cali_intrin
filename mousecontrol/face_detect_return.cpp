#include "stdafx.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

String cascadeName =
"C:/OpenCV2.2/data/haarcascades/haarcascade_frontalface_alt.xml";
String nestedCascadeName =
"C:/OpenCV2.2/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";

extern IplImage *Ipl_rgb;
extern CvPoint face_pos;
extern int face_size;
extern bool face_pos_valid;
extern bool face_detect_enable;
extern int camera_stop;

void face_detect_return( IplImage *iplImg 
                    //, CascadeClassifier& cascade, CascadeClassifier& nestedCascade
                    //, double scale
					//, CvPoint *face_pos, int *face_size
				   )
{
    int i = 0;
    double t = 0;
    vector<Rect> faces;
    const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;

	CascadeClassifier cascade, nestedCascade;
    double scale = 1;
	cascade.load( cascadeName );

	Mat imgOrg, img;
	imgOrg = iplImg;

	//if( iplImg->origin == IPL_ORIGIN_TL )
        imgOrg.copyTo( img );
    //else
    //    flip( imgOrg, img, 0 );

	Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    t = (double)cvGetTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        |CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_ROUGH_SEARCH
        //|CV_HAAR_SCALE_IMAGE
        ,
        Size(30, 30) );
    //t = (double)cvGetTickCount() - t;
    //printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
        //circle( img, center, radius, color, 3, 8, 0 );
        
		face_pos.x = center.x;
		face_pos.y = center.y;
		face_size = radius;
		
/*		
		if( nestedCascade.empty() )
            continue;
        smallImgROI = smallImg(*r);
        nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
            1.1, 2, 0
            //|CV_HAAR_FIND_BIGGEST_OBJECT
            //|CV_HAAR_DO_ROUGH_SEARCH
            //|CV_HAAR_DO_CANNY_PRUNING
            |CV_HAAR_SCALE_IMAGE
            ,
            Size(30, 30) );
        for( vector<Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++ )
        {
            center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
            center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
            radius = cvRound((nr->width + nr->height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
*/
    }  
    //cv::imshow( "result", img );    
}

void face_detect_run ()
{
	face_pos.x = 2;
	face_pos.y = 2; 
	face_size = 2;

	//Mat frame;

	while (face_detect_enable)
	{
		if( camera_stop )
		{
			continue;
		}
		else
		{
			//frame = Ipl_rgb;
            //if( !frame.empty() )
			{
				//face_detect_return( Ipl_rgb_tmp, cascade, nestedCascade, scale, &face_pos, &face_size);
				//face_detect_return( Ipl_rgb, &face_pos, &face_size);
				face_detect_return( Ipl_rgb );
				//cvWaitKey(30); 
			}
		}
	}
}