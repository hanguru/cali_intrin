#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <XnCppWrapper.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>

// OpenGL 관련
#include <gl\glut.h>
#include <gl\GL.h>
#include <gl\GLU.h>

// pthread 관련
#include <pthread.h>
// test... one more

struct cal_data
{
    float depth;
    float length;
};

#define CAL_NUM 10
cal_data h_cal_data[CAL_NUM], v_cal_data[CAL_NUM];
int h_cal_num=0, v_cal_num=0;

void calibration_test(IplImage *Ipl_depth_disp, IplImage *Ipl_calibration_test, int minpoint_x, int minpoint_y, int minpoint_depth, int direction)
{

	//1. display arm only
	//2. measure horizontal and vertical length

	unsigned short *p_Ipl_img_src = (unsigned short *)Ipl_depth_disp->imageData;	
	unsigned char *p_Ipl_img_dst = (unsigned char *)Ipl_calibration_test->imageData;	
	
	for(int yy=0; yy<Ipl_depth_disp->height; yy++){ 
		for(int xx=0; xx<Ipl_depth_disp->width; xx++){ 

			//p_Ipl_img_dst[xx+yy*Ipl_depth_disp->width] = p_Ipl_img_src[xx+yy*Ipl_depth_disp->width];
			if ( (p_Ipl_img_src[xx+yy*Ipl_depth_disp->width] > minpoint_depth + 100) | 
				 (p_Ipl_img_src[xx+yy*Ipl_depth_disp->width] == 0) )
			{
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] = 0;
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] = 0;
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] = 0;
			}
			else
			{
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] = 255;
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] = 255;
				p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] = 255;
			}			
		}
	}

	int left_x=0, left_y=0, right_x=0, right_y=0, top_x=0, top_y=0, bottom_x=0, bottom_y=0;
	CvFont font; 
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 0.5, 0.5, 0,2);
	char s_text[512];

	// Horizontal Length
	if (direction==0)
	{
		for(int xx=0; xx<Ipl_depth_disp->width; xx++){ 
			for(int yy=0; yy<Ipl_depth_disp->height; yy++){ 

				if ( p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] == 255 )
				{
					right_x = xx;
					right_y = yy;
					break;
				}
			}
		}
		for(int xx=Ipl_depth_disp->width-1; xx>=0; xx--){ 
			for(int yy=0; yy<Ipl_depth_disp->height; yy++){ 

				if ( p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] == 255 )
				{
					left_x = xx;
					left_y = yy;
					break;
				}
			}
		}	
	}
	// Vertical Length
	else
	{
		for(int yy=0; yy<Ipl_depth_disp->height; yy++){ 
			for(int xx=0; xx<Ipl_depth_disp->width; xx++){ 
		
				if ( p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] == 255 )
				{
					bottom_x = xx;
					bottom_y = yy;
					break;
				}
			}
		}
		for(int yy=Ipl_depth_disp->height-1; yy>=0; yy--){ 
			for(int xx=0; xx<Ipl_depth_disp->width; xx++){ 
		
				if ( p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +0] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +1] == 255 &
					p_Ipl_img_dst[3*(xx+yy*Ipl_depth_disp->width) +2] == 255 )
				{
					top_x = xx;
					top_y = yy;
					break;
				}
			}
		}	
	}

	if (direction==0)
	{
		int left_depth = p_Ipl_img_src[left_x+left_y*Ipl_depth_disp->width];
		int right_depth = p_Ipl_img_src[right_x+right_y*Ipl_depth_disp->width];

		sprintf( s_text, "%d",left_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(left_x, left_y), &font, CV_RGB(255,0,0));
		sprintf( s_text, "%d",right_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(right_x, right_y), &font, CV_RGB(255,0,0));

		if( (left_y == right_y)&(left_y != 0) & (h_cal_num<CAL_NUM) )
		{
			cvLine(Ipl_calibration_test, cvPoint(left_x, left_y), cvPoint(right_x, right_y), CV_RGB(255,0,0), 2, 4, 0);

			if (left_depth==right_depth)
			{
				h_cal_data[h_cal_num].depth = left_depth;
				h_cal_data[h_cal_num].length = right_x - left_x;
				h_cal_num ++;
			}
		}
	}
	else
	{
		int top_depth = p_Ipl_img_src[top_x+top_y*Ipl_depth_disp->width];
		int bottom_depth = p_Ipl_img_src[bottom_x+bottom_y*Ipl_depth_disp->width];

		sprintf( s_text, "%d",top_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(top_x, top_y), &font, CV_RGB(255,0,0));
		sprintf( s_text, "%d",bottom_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(bottom_x, bottom_y), &font, CV_RGB(255,0,0));

		if( (top_x == bottom_x)&(top_x != 0)& (v_cal_num<CAL_NUM))
		{
			cvLine(Ipl_calibration_test, cvPoint(top_x, top_y), cvPoint(bottom_x, bottom_y), CV_RGB(255,0,0), 2, 4, 0);

			if (top_depth==bottom_depth) 
			{
				v_cal_data[v_cal_num].depth = top_depth;
				v_cal_data[v_cal_num].length = bottom_y - top_y;
				v_cal_num ++;
			}
		}
	}


	//sprintf( s_text, "min_x=%d, min_y=%d", minpoint_x, minpoint_y);
	
	sprintf( s_text, "left_x=%d, left_y=%d, right_x=%d, right_y=%d H_length = %d", left_x, left_y, right_x, right_y, right_x-left_x);
	cvPutText (Ipl_calibration_test, s_text,cvPoint(10,30), &font, cvScalar(255,255,0));

	sprintf( s_text, "top_x=%d, top_y=%d, bottom_x=%d, bottom_y=%d V_length = %d", top_x, top_y, bottom_x, bottom_y, bottom_y-top_y);
	cvPutText (Ipl_calibration_test, s_text,cvPoint(10,60), &font, cvScalar(255,255,0));

	for (int i =0; i<h_cal_num; i++)
	{
		sprintf( s_text, "%d. depth=%d, length=%d", i, (int)h_cal_data[i].depth, (int)h_cal_data[i].length);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(10,90+30*i), &font, cvScalar(255,255,0));
	}
	for (int i =0; i<v_cal_num; i++)
	{
		sprintf( s_text, "%d. depth=%d, length=%d", i, (int)v_cal_data[i].depth, (int)v_cal_data[i].length);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(320,90+30*i), &font, cvScalar(255,255,0));
	}

}
