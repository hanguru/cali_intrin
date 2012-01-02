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

#define CAL_NUM 100
cal_data h_cal_data[CAL_NUM], v_cal_data[CAL_NUM];
int h_cal_num=0, v_cal_num=0, focal_x_num=0, focal_y_num=0;
CvPoint left_pt[CAL_NUM], right_pt[CAL_NUM], top_pt[CAL_NUM], bottom_pt[CAL_NUM];
float focal_x[CAL_NUM*CAL_NUM], focal_y[CAL_NUM*CAL_NUM];

float left_x_ave=0, left_y_ave=0, right_x_ave=0, right_y_ave=0, left_depth_ave=0, right_depth_ave=0;
float top_x_ave=0, top_y_ave=0, bottom_x_ave=0, bottom_y_ave=0, top_depth_ave=0, bottom_depth_ave=0;

int focal_test_num=0;

void calibration_test(IplImage *Ipl_depth_disp, IplImage *Ipl_calibration_test, int minpoint_x, int minpoint_y, int minpoint_depth, int direction)
{

	//1. display arm only
	//2. measure horizontal and vertical length

	unsigned short *p_Ipl_img_src = (unsigned short *)Ipl_depth_disp->imageData;	
	unsigned char *p_Ipl_img_dst = (unsigned char *)Ipl_calibration_test->imageData;	
	
	for(int yy=0; yy<Ipl_depth_disp->height; yy++){ 
		for(int xx=0; xx<Ipl_depth_disp->width; xx++){ 

			//p_Ipl_img_dst[xx+yy*Ipl_depth_disp->width] = p_Ipl_img_src[xx+yy*Ipl_depth_disp->width];
			if ( (p_Ipl_img_src[xx+yy*Ipl_depth_disp->width] > minpoint_depth + 400) | 
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

	// horizontal direction
	if (direction==0)
	{
		int left_depth, right_depth;
#if 1
		// focal length test
		int x1[36] = {-256, -187, -187, -153, -130, -217, -62, -159, -117, -203, -184, -216, -230, -215, -150, -228, -238, -172, -226, -204, -101, -129, -198, -202, -202, -117, -114, -187, -166, -70, -262, -137, -269, -177, -260, -141};
		int z1[36] = {823, 693, 483, 781, 881, 668, 976, 833, 1009, 678, 850, 644, 701, 662, 927, 638, 867, 940, 611, 723, 827, 625, 846, 724, 888, 863, 1213, 652, 556, 817, 646, 666, 885, 534, 582, 963};
		int x2[36] = {86, 171, 193, 215, 193, 207, 222, 187, 156, 213, 152, 222, 62, 210, 159, 221, 94, 134, 47, 187, 250, 159, 148, 198, 127, 219, 108, 68, 148, 140, 24, 94, 50, 126, 21, 145};
		int z2[36] = {836,  830,  753,  759,  873,  668,  1002,  805,  1053,  685,  838,  652,  1003,  674,  911,  627,  810,  915,  985,  735,  805,  922,  777,  701,  794,  842,  947,  1019,  870,  1140,  1012,  1040,  951,  884,  983,  1039};
				
		// fake sample...
		if (focal_test_num < 36)
		{
			left_x = x1[focal_test_num] + 320;
			right_x = x2[focal_test_num] + 320;
			left_y = right_y = 100;
			left_depth = z1[focal_test_num];
			right_depth = z2[focal_test_num];
			focal_test_num++;
		}
		
		if (focal_test_num == 36)
		{
			focal_test_num = focal_test_num;
		}
#else
		left_depth = p_Ipl_img_src[left_x+left_y*Ipl_depth_disp->width];
		right_depth = p_Ipl_img_src[right_x+right_y*Ipl_depth_disp->width];

		float ave_rate = 0.1;
		left_x_ave = left_x_ave * (1-ave_rate) + left_x * ave_rate;
		left_y_ave = left_y_ave * (1-ave_rate) + left_y * ave_rate;
		left_depth_ave = left_depth_ave * (1-ave_rate) + left_depth * ave_rate;
		right_x_ave = right_x_ave * (1-ave_rate) + right_x * ave_rate;
		right_y_ave = right_y_ave * (1-ave_rate) + right_y * ave_rate;
		right_depth_ave = right_depth_ave * (1-ave_rate) + right_depth * ave_rate;

		left_x = (int)left_x_ave;
		left_y = (int)left_y_ave;
		left_depth = (int)left_depth_ave;
		right_x = (int)right_x_ave;
		right_y = (int)right_y_ave;
		right_depth = (int)right_depth_ave;
#endif

		sprintf( s_text, "%d",left_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(left_x, left_y), &font, CV_RGB(255,0,0));
		sprintf( s_text, "%d",right_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(right_x, right_y), &font, CV_RGB(255,0,0));

		//if( (left_y == right_y)&(left_y != 0) & (h_cal_num<CAL_NUM))
		if( (abs(left_y - right_y) < 2) & (left_y != 0) & (h_cal_num<CAL_NUM))
		{
			if ( (h_cal_num == 0) | ((abs(left_pt[h_cal_num-1].x - left_x) > 10) & (abs(right_pt[h_cal_num-1].x - right_x) > 10)) )
			{
				
				cvLine(Ipl_calibration_test, cvPoint(left_x, left_y), cvPoint(right_x, right_y), CV_RGB(255,0,0), 2, 4, 0);

				//if (left_depth==right_depth)
				if(1)
				{
					if (h_cal_num!=0)
					{
						unsigned int c = abs((left_x-320) * left_depth - (right_x-320) * right_depth);
						unsigned int d = abs(left_depth - right_depth);
					
						//FILE *cal_focal;
						//cal_focal = fopen ("calibration_fxfy.dat", "a+");
						//fprintf (cal_focal, "%d\t%d\t%d\t%d\n", left_x-320, left_depth, right_x-320, right_depth);
						//fclose(cal_focal);

						for (int i =0; i< h_cal_num; i++)
						{
							unsigned int a = abs((left_pt[i].x-320) * left_pt[i].y - (right_pt[i].x-320) * right_pt[i].y);
							unsigned int b = abs(left_pt[i].y - right_pt[i].y);

							// calculation results are valid only for samples at different distances.
							// calculation between nearby samples increase noise(error).
							//if ( ((left_pt[i].y - left_depth) > 100) || ((left_pt[i].y - left_depth) < -100) )
							if ( (abs((float)a-(float)c) > 10000 ) & (abs((float)d-(float)b) > 100) & 
								(((float)a-(float)c) * ((float)d-(float)b) > 0 ) )
							{

								float fx_square = (d*d-b*b)==0 ? 0 : ((float)a*a-(float)c*c)/((float)d*d-(float)b*b);

								// calculation results from samples with small difference of left right distance
								// can cause minus fx_square result or increased noise result. 
								if (fx_square < 0)
									fx_square = fx_square;

								FILE *cal_focal;
								cal_focal = fopen ("calibration_fxfy.dat", "a+");
								//fprintf (cal_focal, "%d\t%d\t%d\t%d\t%f\n", a, b, c, d, fx_square);
								fprintf (cal_focal, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n", left_x-320, left_depth, right_x-320, right_depth, left_pt[i].x-320, left_pt[i].y, right_pt[i].x-320, right_pt[i].y, fx_square, sqrt(fx_square));
								fclose(cal_focal);

								focal_x[focal_x_num] = sqrt(fx_square);
								focal_x_num++;
							}
						}
					}

					left_pt[h_cal_num].x = left_x;
					left_pt[h_cal_num].y = left_depth;
					right_pt[h_cal_num].x = right_x;
					right_pt[h_cal_num].y = right_depth;

					h_cal_data[h_cal_num].depth = left_depth;
					h_cal_data[h_cal_num].length = right_x - left_x;
					h_cal_num ++;
				}
			}
		}
	}
	// vertical direction
	else
	{
		int top_depth, bottom_depth;

#if 1
		// focal length test
		int y1[41] = {-117, -58, -166, -135, -104, -119, -196, -185, -185, -152, -165, -207, -202, -149, -209, -142, -101, -196, -197, -147, -192, -142, -130, -102, -180, -50, -113, -218, -164, -139, -185, -158, -51, -85, -157, -124, -122, -179, -177, -85, -178};
		int z1[41] = {1081, 1123, 905, 1131, 1124, 904, 696, 776, 825, 966, 935, 754, 715, 992, 691, 1041, 1122, 749, 783, 637, 748, 1052, 1114, 1037, 807, 942, 1152, 704, 1001, 786, 586, 1080, 902, 985, 825, 1029, 918, 815, 673, 1069, 651};
		int y2[41] = {136, 113, 144, 111, 139, 88, 197, 176, 153, 137, 134, 162, 183, 129, 193, 127, 151, 176, 164, 128, 176, 125, 123, 100, 168, 139, 119, 176, 116, 83, 114, 104, 123, 111, 74, 69, 102, 62, 212, 95, 106};
		int z2[41] = {1120, 1411, 904, 1150, 1169, 1216, 731, 783, 841, 982, 949, 774, 748, 1033, 713, 1059, 1121, 771, 784, 953, 787, 1069, 1128, 1308, 825, 1250, 1249, 740, 1033, 1130, 913, 1097, 1252, 1284, 1150, 1343, 1200, 1138, 765, 1381, 970};
				
		// fake sample...
		if (focal_test_num < 41)
		{
			top_y = y1[focal_test_num] + 240;
			bottom_y = y2[focal_test_num] + 240;
			top_x = bottom_x = 100;
			top_depth = z1[focal_test_num];
			bottom_depth = z2[focal_test_num];
			focal_test_num++;
		}
		
		if (focal_test_num == 41)
		{
			focal_test_num = focal_test_num;
		}


#else
		top_depth = p_Ipl_img_src[top_x+top_y*Ipl_depth_disp->width];
		bottom_depth = p_Ipl_img_src[bottom_x+bottom_y*Ipl_depth_disp->width];

		float ave_rate = 0.1;
		top_x_ave = top_x_ave * (1-ave_rate) + top_x * ave_rate;
		top_y_ave = top_y_ave * (1-ave_rate) + top_y * ave_rate;
		top_depth_ave = top_depth_ave * (1-ave_rate) + top_depth * ave_rate;
		bottom_x_ave = bottom_x_ave * (1-ave_rate) + bottom_x * ave_rate;
		bottom_y_ave = bottom_y_ave * (1-ave_rate) + bottom_y * ave_rate;
		bottom_depth_ave = bottom_depth_ave * (1-ave_rate) + bottom_depth * ave_rate;

		top_x = (int)top_x_ave;
		top_y = (int)top_y_ave;
		top_depth = (int)top_depth_ave;
		bottom_x = (int)bottom_x_ave;
		bottom_y = (int)bottom_y_ave;
		bottom_depth = (int)bottom_depth_ave;
#endif

		sprintf( s_text, "%d",top_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(top_x, top_y), &font, CV_RGB(255,0,0));
		sprintf( s_text, "%d",bottom_depth);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(bottom_x, bottom_y), &font, CV_RGB(255,0,0));

		if( (abs(top_x - bottom_x)<2)&(top_x != 0) & (v_cal_num<CAL_NUM))
		{
			if ( (v_cal_num == 0) | ((abs(top_pt[v_cal_num-1].x - top_y) > 10) & (abs(bottom_pt[v_cal_num-1].x - bottom_y) > 10)) )
			{
				
				cvLine(Ipl_calibration_test, cvPoint(top_x, top_y), cvPoint(bottom_x, bottom_y), CV_RGB(255,0,0), 2, 4, 0);

				//if (top_depth==bottom_depth)
				if(1)
				{
					if (v_cal_num!=0)
					{
						unsigned int c = abs((top_y-240) * top_depth - (bottom_y-240) * bottom_depth);
						unsigned int d = abs(top_depth - bottom_depth);

						//FILE *cal_focal;
						//cal_focal = fopen ("calibration_fxfy.dat", "a+");
						//fprintf (cal_focal, "%d\t%d\t%d\t%d\n", top_y-240, top_depth, bottom_y-240, bottom_depth);
						//fclose(cal_focal);


						for (int i =0; i< v_cal_num; i++)
						{
							unsigned int a = abs((top_pt[i].x-240) * top_pt[i].y - (bottom_pt[i].x-240) * bottom_pt[i].y);
							unsigned int b = abs(top_pt[i].y - bottom_pt[i].y);

							// calculation results are valid only for samples at different distances.
							// calculation between nearby samples increase noise(error).
							//if ( ((top_pt[i].y - top_depth) > 100) || ((top_pt[i].y - top_depth) < -100) )
							if ( (abs((float)a-(float)c) > 10000 ) & (abs((float)d-(float)b) > 100) & 
								(((float)a-(float)c) * ((float)d-(float)b) > 0 ) )
							{

								float fx_square = (d*d-b*b)==0 ? 0 : ((float)a*a-(float)c*c)/((float)d*d-(float)b*b);

								// calculation results from samples with small difference of top bottom distance
								// can cause minus fx_square result or increased noise result. 
								if (fx_square < 0)
									fx_square = fx_square;

								FILE *cal_focal;
								cal_focal = fopen ("calibration_fxfy.dat", "a+");
								//fprintf (cal_focal, "%d\t%d\t%d\t%d\t%f\n", a, b, c, d, fx_square);
								fprintf (cal_focal, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n", top_y-240, top_depth, bottom_y-240, bottom_depth, top_pt[i].x-240, top_pt[i].y, bottom_pt[i].x-240, bottom_pt[i].y, fx_square, sqrt(fx_square));
								fclose(cal_focal);

								focal_y[focal_y_num] = sqrt(fx_square);
								focal_y_num++;
							}
						}
					}

					top_pt[v_cal_num].x = top_y;
					top_pt[v_cal_num].y = top_depth;
					bottom_pt[v_cal_num].x = bottom_y;
					bottom_pt[v_cal_num].y = bottom_depth;

					v_cal_data[v_cal_num].depth = top_depth;
					v_cal_data[v_cal_num].length = bottom_y - top_y;
					v_cal_num ++;
				}
			}
		}

/*
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
*/
	}


	//sprintf( s_text, "min_x=%d, min_y=%d", minpoint_x, minpoint_y);
	
	float focal_x_ave=0;
	if ( focal_x_num > 0 & focal_x_num <= CAL_NUM*CAL_NUM)
	{
		for (int i=0; i<focal_x_num; i++)
		{
			focal_x_ave += focal_x[i];
		}
		focal_x_ave /= focal_x_num;
	}

	sprintf( s_text, "focal_x=%f left_x=%d, left_y=%d, right_x=%d, right_y=%d H_length = %d", focal_x_ave, left_x, left_y, right_x, right_y, right_x-left_x);
	cvPutText (Ipl_calibration_test, s_text,cvPoint(10,30), &font, cvScalar(255,255,0));

	float focal_y_ave=0;
	if ( focal_y_num > 0 & focal_y_num <= CAL_NUM*CAL_NUM)
	{
		for (int i=0; i<focal_y_num; i++)
		{
			focal_y_ave += focal_y[i];
		}
		focal_y_ave /= focal_y_num;
	}

	sprintf( s_text, "focal_y=%f top_x=%d, top_y=%d, bottom_x=%d, bottom_y=%d V_length = %d", focal_y_ave, top_x, top_y, bottom_x, bottom_y, bottom_y-top_y);
	cvPutText (Ipl_calibration_test, s_text,cvPoint(10,60), &font, cvScalar(255,255,0));

	int cal_st;
	if (h_cal_num > 10) cal_st = h_cal_num - 10;
	else cal_st = 0;
	//int cal_st = h_cal_num - (h_cal_num%10);
	for (int i =cal_st; i<h_cal_num; i++)
	//for (int i =0; i<h_cal_num; i++)
	{
		//sprintf( s_text, "%d. depth=%d, length=%d", i, (int)h_cal_data[i].depth, (int)h_cal_data[i].length);
		sprintf( s_text, "left_x=%d. left_depth=%d, right_x=%d. right_depth=%d", (int)left_pt[i].x, (int)left_pt[i].y, (int)right_pt[i].x, (int)right_pt[i].y);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(10,90+30*(i-cal_st)), &font, cvScalar(255,255,0));
	}

	if (v_cal_num > 10) cal_st = v_cal_num - 10;
	else cal_st = 0;
	//cal_st = v_cal_num - (v_cal_num%10);
	for (int i =cal_st; i<v_cal_num; i++)
	//for (int i =0; i<v_cal_num; i++)
	{
		//sprintf( s_text, "%d. depth=%d, length=%d", i, (int)v_cal_data[i].depth, (int)v_cal_data[i].length);
		//cvPutText (Ipl_calibration_test, s_text,cvPoint(320,90+30*(i-cal_st)), &font, cvScalar(255,255,0));
		sprintf( s_text, "top_x=%d. top_depth=%d, bottom_x=%d. bottom_depth=%d", (int)top_pt[i].x, (int)top_pt[i].y, (int)bottom_pt[i].x, (int)bottom_pt[i].y);
		cvPutText (Ipl_calibration_test, s_text,cvPoint(10,90+30*(i-cal_st)), &font, cvScalar(255,255,0));

	}

}
