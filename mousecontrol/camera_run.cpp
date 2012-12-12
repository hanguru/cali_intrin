//OPENCV를 이용하기 위해서 OPENNI 상에서 받은 RGB와 DEPTH영상을 IplImage로 전환하는 과정이다.
//이렇게 전환 하고 나면 각 픽셀 에서의 DEPTH 크기 및 각 픽셀의 RGB 값을 얻을 수 있다. 

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

#define SAMPLE_XML_PATH "C:/Program Files/OpenNI/Data/SamplesConfig.xml"

#include "PostureClassifier.h"

using namespace xn;

Context g_context;
DepthGenerator g_depth;
ImageGenerator g_image;
const XnDepthPixel* ppDepth=NULL;
//BYTE b,g,r;

extern IplImage *Ipl_rgb, *Ipl_depth, *Ipl_rgb2, *Ipl_depth_warp;

extern cv::Point2f warp_src_tl, warp_src_bl, warp_src_br, warp_src_tr;
extern cv::Point2f warp_dst_tl, warp_dst_bl, warp_dst_br, warp_dst_tr;

extern int camera_update, camera_stop;

extern int minpoint_x, minpoint_y, minpoint_depth, maxdepth;

extern int warp_enable;

extern int WIN_SIZE_X;
extern int WIN_SIZE_Y;

IplImage * Ipl_rgb2_temp;

extern bool PROG_EXIT;

extern void hmi_center (int x_pos, int y_pos, int z_pos,
			float body_pos_x, float body_pos_y, float body_float_z, int face_size, int hand_palm);
//extern void hmi_center_relative (int x_pos, int y_pos, int z_pos,
//			float body_pos_x, float body_pos_y, float body_float_z, int face_size, int hand_palm);
//extern void hmi_center_absolute (int x_pos, int y_pos, int z_pos);
extern bool abs_mouse_con, rel_mouse_con;

extern int mouse_state;
int file_num=0;

extern bool face_detect_enable;
extern void face_detect_return(IplImage *iplImg, CvPoint *face_pos, int *face_size);

extern CvPoint face_pos;
extern int face_size;
extern bool face_pos_valid;

extern int scrol_state;

extern void calibration_test(IplImage *Ipl_depth_disp, IplImage *Ipl_calibration_test, int minpoint_x, int minpoint_y, int minpoint_depth, int direction);
extern bool arm_horizontal_measure, arm_vertical_measure;

// Human machine interface 
#define MOUSE_IDLE 0
#define MOUSE_RIGHT_PUSH 1
#define MOUSE_LEFT_PUSH 2
#define MOUSE_CLICK_N_MOVE 3
#define MOUSE_READY 4
#define MOUSE_RIGHT_POP 5
#define MOUSE_LEFT_POP 6
extern int mouse_state;

extern bool depth_updating, rgb_updating, depth_copying;

extern void mouse_click(int click_type);
	
int minpoint_x_current, minpoint_y_current, minpoint_depth_current;

void draw_rect (IplImage *Ipl_img, int x, int y, int size, int data_bit)
{
	unsigned short *p_Ipl_img_short = (unsigned short *)Ipl_img->imageData;	
	unsigned char *p_Ipl_img_char = (unsigned char *)Ipl_img->imageData;	
	
	for(int yy=y-size; yy<y+size; yy++){
		for(int xx=x-size; xx<x+size; xx++){
			if ( (xx >= 0 && xx < WIN_SIZE_X ) && (yy >= 0 && yy < WIN_SIZE_Y ) )
			{
				if ( data_bit == 16) 
				{
					p_Ipl_img_short[xx+yy*WIN_SIZE_X]=256*256-1;
				}
				else 
				{
					p_Ipl_img_char[3*(xx+yy*WIN_SIZE_X)+0]=0;
					p_Ipl_img_char[3*(xx+yy*WIN_SIZE_X)+1]=0;
					p_Ipl_img_char[3*(xx+yy*WIN_SIZE_X)+2]=255;
				}
			}
		}
	}
}


void Depth_to_IplImage (IplImage* image, xn::DepthGenerator g_depth,const XnDepthPixel* ppDepth, DepthMetaData* g_depthMD)
{
	
	//DepthMetaData g_depthMD;
	
	//int tempmax=0, tempmin_x=0, tempmin_y=0, tempmin=256*256-1;

	//XnUInt32 nMiddleIndex = XN_VGA_X_RES * XN_VGA_Y_RES/2 +  XN_VGA_X_RES/2;
	
	//g_depth.GetMetaData(g_depthMD);

	ppDepth = g_depthMD->Data();

	// XN_PIXEL_FORMAT_RGB24
	//*tempmid=ppDepth[nMiddleIndex];

	//unsigned short *p_imageData = (unsigned short *)image->imageData;
	unsigned short *p_imageData = (unsigned short *)image->imageData;
	//unsigned short *p_imageData_disp = (unsigned short *)image_disp->imageData;
	
	//for(int xx=0; xx<WIN_SIZE_X*WIN_SIZE_Y; xx++){
	/*
	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
		
			if(ppDepth[xx+yy*WIN_SIZE_X]>tempmax)
			{
				tempmax=ppDepth[xx+yy*WIN_SIZE_X];
			}
			if( (ppDepth[xx+yy*WIN_SIZE_X]<tempmin) &&
				(ppDepth[xx+yy*WIN_SIZE_X]>10) )
			{
				tempmin=ppDepth[xx+yy*WIN_SIZE_X]; 
				tempmin_x = WIN_SIZE_X -1 - xx;
				tempmin_y = yy;
			}
		}
	}
	*/

	while (depth_copying) {}

	depth_updating = 1;

	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
			int xx_inv = WIN_SIZE_X -1 - xx;
/*			
			image->imageData[3*(xx+yy*WIN_SIZE_X)+0]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			image->imageData[3*(xx+yy*WIN_SIZE_X)+1]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			image->imageData[3*(xx+yy*WIN_SIZE_X)+2]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			
			if(ppDepth[xx_inv+yy*WIN_SIZE_X]==0){
				image->imageData[3*(xx+yy*WIN_SIZE_X)+0]=0;
				image->imageData[3*(xx+yy*WIN_SIZE_X)+1]=0;
				image->imageData[3*(xx+yy*WIN_SIZE_X)+2]=0;
			}
*/
			p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
			//p_imageData_disp[xx+yy*WIN_SIZE_X] = (unsigned short)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*(256*256-1));
			
/*			
			if ( ppDepth[xx_inv+yy*WIN_SIZE_X] != 0)
			{
				//p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)(((256*256-1) - (float)ppDepth[xx_inv+yy*WIN_SIZE_X])/(tempmax)*(256*256-1));
				p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
			}
			else
			{
				if (xx == 0 ) 
					p_imageData[xx+yy*WIN_SIZE_X] = 255;
				else 
					p_imageData[xx+yy*WIN_SIZE_X] = p_imageData[xx-1+yy*WIN_SIZE_X];
				//p_imageData[xx+yy*WIN_SIZE_X] = 0;
			}
*/			
			//p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
		}
	}
	
	depth_updating = 0;

/*	
	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
		
			if(p_imageData[xx+yy*WIN_SIZE_X]>tempmax)
			{
				tempmax=p_imageData[xx+yy*WIN_SIZE_X];
			}
			if( (p_imageData[xx+yy*WIN_SIZE_X]<tempmin) &&
				(p_imageData[xx+yy*WIN_SIZE_X]>10) )
			{
				tempmin=p_imageData[xx+yy*WIN_SIZE_X]; 
				tempmin_x = xx;
				tempmin_y = yy;
			}
		}
	}

	minpoint_x_current = tempmin_x;
	minpoint_y_current = tempmin_y;
	minpoint_depth_current = tempmin;

	//if ( (tempmin >900) & (tempmin < 1100))

	// find the finger tip by image processingK
	int search_win_size = 50;
	int search_depth_size = 50;
	if (tempmin_x < search_win_size) tempmin_x = search_win_size;
	else if (tempmin_x > 640-search_win_size) tempmin_x = 640 - search_win_size;
	if (tempmin_y < search_win_size) tempmin_y = search_win_size;
	else if (tempmin_y > 480-search_win_size) tempmin_y = 480 - search_win_size;

	int sub_tempmin_x = tempmin_x;
	int sub_tempmin_y = tempmin_y;
	int loop_break = 0;
	for(int yy=sub_tempmin_y-search_win_size; yy<sub_tempmin_y+search_win_size; yy++){ 
		for(int xx=sub_tempmin_x-search_win_size; xx<sub_tempmin_x+search_win_size; xx++){ 

			if( (p_imageData[xx+yy*WIN_SIZE_X] != 0) & (p_imageData[xx+yy*WIN_SIZE_X] > 200) &
				(p_imageData[xx+yy*WIN_SIZE_X] < tempmin + search_depth_size) )
			{
				tempmin=p_imageData[xx+yy*WIN_SIZE_X]; 
				tempmin_x = xx;
				tempmin_y = yy;
				loop_break = 1;
				break;
			}
		}
		if (loop_break) break;
	}


	if (abs_mouse_con)
	{
		*maxdepth = tempmax;
		*minpoint_depth = tempmin;
		*minpoint_x = 0.7*(*minpoint_x) + 0.3*tempmin_x;
		*minpoint_y = 0.7*(*minpoint_y) + 0.3*tempmin_y;
	}
	else
	{
		*maxdepth = tempmax;
		*minpoint_depth = tempmin;
		*minpoint_x = tempmin_x;
		*minpoint_y = tempmin_y;
	}
*/

	//*minpoint_x = tempmin_x;
	//*minpoint_y = tempmin_y;
	//*minpoint_depth = tempmin;
	//draw_rect(image, *minpoint_x, *minpoint_y, 3, 16);
	
	//*maxdepth = tempmax;
	//*minpoint_x = 0.7*(*minpoint_x) + 0.3*tempmin_x;
	//*minpoint_y = 0.7*(*minpoint_y) + 0.3*tempmin_y;
	//*minpoint_depth = 0.7*(*minpoint_depth) + 0.3*tempmin;
	//*minpoint_x = tempmin_x;
	//*minpoint_y = tempmin_y;

	//minpoint_x_current = tempmin_x;
	//minpoint_y_current = tempmin_y;
	//minpoint_depth_current = tempmin;

	//draw_rect(image, *minpoint_x, *minpoint_y, 3, 16);
	//draw_rect(image_disp, *minpoint_x, *minpoint_y, 3, 16);
	//return ppDepth;

/*
	// image file dump
	stringstream ss;
	ss<<file_num;
	ss<<".xml";
	file_num ++;
	
	cv::FileStorage File(ss.str().c_str(),cv::FileStorage::WRITE);
	cv::Mat b(image);
	cv::write(File,"mat",b);
	File.release();
*/
}

void RGB_to_IplImage (IplImage* image,xn::ImageGenerator g_image, ImageMetaData* g_imageMD)
{
	//ImageMetaData g_imageMD;


	//g_image.GetMetaData(g_imageMD);

	const XnRGB24Pixel* pImageRow = g_imageMD->RGB24Data();

	//XN_PIXEL_FORMAT_RGB24

	rgb_updating = 1;

	for(int yy=0; yy<WIN_SIZE_Y; yy++){
		for(int xx=0; xx<WIN_SIZE_X; xx++){
			int xx_inv = WIN_SIZE_X -1 - xx;
			image->imageData[3*(xx+yy*WIN_SIZE_X)+0]=pImageRow[xx_inv+yy*WIN_SIZE_X].nBlue;
			image->imageData[3*(xx+yy*WIN_SIZE_X)+1]=pImageRow[xx_inv+yy*WIN_SIZE_X].nGreen;
			image->imageData[3*(xx+yy*WIN_SIZE_X)+2]=pImageRow[xx_inv+yy*WIN_SIZE_X].nRed;
		}
	}
	
	rgb_updating = 0;

	//return 0;
}

void get_Depth_minpoint (IplImage* image, IplImage* image_disp, int* maxdepth, 
					int* minpoint_depth,int* minpoint_x, int* minpoint_y)
{
	
	//DepthMetaData g_depthMD;
	
	int tempmax=0, tempmin_x=0, tempmin_y=0, tempmin=256*256-1;

	//XnUInt32 nMiddleIndex = XN_VGA_X_RES * XN_VGA_Y_RES/2 +  XN_VGA_X_RES/2;
	
	//g_depth.GetMetaData(g_depthMD);

	//ppDepth = g_depthMD->Data();

	// XN_PIXEL_FORMAT_RGB24
	//*tempmid=ppDepth[nMiddleIndex];

	//unsigned short *p_imageData = (unsigned short *)image->imageData;
	unsigned short *p_imageData = (unsigned short *)image->imageData;
	unsigned short *p_imageData_disp = (unsigned short *)image_disp->imageData;
	
	//for(int xx=0; xx<WIN_SIZE_X*WIN_SIZE_Y; xx++){
/*
	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
		
			if(p_imageData[xx+yy*WIN_SIZE_X]>tempmax)
			{
				tempmax=p_imageData[xx+yy*WIN_SIZE_X];
			}
			if( (p_imageData[xx+yy*WIN_SIZE_X]<tempmin) &&
				(p_imageData[xx+yy*WIN_SIZE_X]>10) )
			{
				tempmin=p_imageData[xx+yy*WIN_SIZE_X]; 
				tempmin_x = WIN_SIZE_X -1 - xx;
				tempmin_y = yy;
			}
		}
	}
*/
	depth_copying = 1;

	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
			//int xx_inv = WIN_SIZE_X -1 - xx;
/*			
			image->imageData[3*(xx+yy*WIN_SIZE_X)+0]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			image->imageData[3*(xx+yy*WIN_SIZE_X)+1]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			image->imageData[3*(xx+yy*WIN_SIZE_X)+2]=255-(unsigned char)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*255);
			
			if(ppDepth[xx_inv+yy*WIN_SIZE_X]==0){
				image->imageData[3*(xx+yy*WIN_SIZE_X)+0]=0;
				image->imageData[3*(xx+yy*WIN_SIZE_X)+1]=0;
				image->imageData[3*(xx+yy*WIN_SIZE_X)+2]=0;
			}
*/
			//p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
			p_imageData_disp[xx+yy*WIN_SIZE_X] = p_imageData[xx+yy*WIN_SIZE_X];///(tempmax)*(256*256-1);
			//(unsigned short)((float)ppDepth[xx_inv+yy*WIN_SIZE_X]/(tempmax)*(256*256-1));
			
/*			
			if ( ppDepth[xx_inv+yy*WIN_SIZE_X] != 0)
			{
				//p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)(((256*256-1) - (float)ppDepth[xx_inv+yy*WIN_SIZE_X])/(tempmax)*(256*256-1));
				p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
			}
			else
			{
				if (xx == 0 ) 
					p_imageData[xx+yy*WIN_SIZE_X] = 255;
				else 
					p_imageData[xx+yy*WIN_SIZE_X] = p_imageData[xx-1+yy*WIN_SIZE_X];
				//p_imageData[xx+yy*WIN_SIZE_X] = 0;
			}
*/			
			//p_imageData[xx+yy*WIN_SIZE_X] = (unsigned short)ppDepth[xx_inv+yy*WIN_SIZE_X];
		}
	}

	depth_copying = 0;
	
	for(int yy=0; yy<WIN_SIZE_Y; yy++){ 
		for(int xx=0; xx<WIN_SIZE_X; xx++){ 
		
			if(p_imageData_disp[xx+yy*WIN_SIZE_X]>tempmax)
			{
				tempmax=p_imageData_disp[xx+yy*WIN_SIZE_X];
			}
			if( (p_imageData_disp[xx+yy*WIN_SIZE_X]<tempmin) &&
				(p_imageData_disp[xx+yy*WIN_SIZE_X]>10) )
			{
				tempmin=p_imageData_disp[xx+yy*WIN_SIZE_X]; 
				tempmin_x = xx;
				tempmin_y = yy;
			}
		}
	}

	minpoint_x_current = tempmin_x;
	minpoint_y_current = tempmin_y;
	minpoint_depth_current = tempmin;

	//if ( (tempmin >900) & (tempmin < 1100))

	// find the finger tip by image processingK
	int search_win_size = 50;
	int search_depth_size = 50;
	if (tempmin_x < search_win_size) tempmin_x = search_win_size;
	else if (tempmin_x > 640-search_win_size) tempmin_x = 640 - search_win_size;
	if (tempmin_y < search_win_size) tempmin_y = search_win_size;
	else if (tempmin_y > 480-search_win_size) tempmin_y = 480 - search_win_size;

	int sub_tempmin_x = tempmin_x;
	int sub_tempmin_y = tempmin_y;
	int loop_break = 0;
	for(int yy=sub_tempmin_y-search_win_size; yy<sub_tempmin_y+search_win_size; yy++){ 
		for(int xx=sub_tempmin_x-search_win_size; xx<sub_tempmin_x+search_win_size; xx++){ 

			if( (p_imageData_disp[xx+yy*WIN_SIZE_X] != 0) & (p_imageData_disp[xx+yy*WIN_SIZE_X] > 200) &
				(p_imageData_disp[xx+yy*WIN_SIZE_X] < tempmin + search_depth_size) )
			{
				tempmin=p_imageData_disp[xx+yy*WIN_SIZE_X]; 
				tempmin_x = xx;
				tempmin_y = yy;
				loop_break = 1;
				break;
			}
		}
		if (loop_break) break;
	}


	if (abs_mouse_con)
	{
		*maxdepth = tempmax;
		*minpoint_depth = tempmin;
		*minpoint_x = 0.7*(*minpoint_x) + 0.3*tempmin_x;
		*minpoint_y = 0.7*(*minpoint_y) + 0.3*tempmin_y;
	}
	else
	{
		*maxdepth = tempmax;
		*minpoint_depth = tempmin;
		*minpoint_x = tempmin_x;
		*minpoint_y = tempmin_y;
	}
	//*minpoint_x = tempmin_x;
	//*minpoint_y = tempmin_y;
	//*minpoint_depth = tempmin;
	//draw_rect(image, *minpoint_x, *minpoint_y, 3, 16);
	
	//*maxdepth = tempmax;
	//*minpoint_x = 0.7*(*minpoint_x) + 0.3*tempmin_x;
	//*minpoint_y = 0.7*(*minpoint_y) + 0.3*tempmin_y;
	//*minpoint_depth = 0.7*(*minpoint_depth) + 0.3*tempmin;
	//*minpoint_x = tempmin_x;
	//*minpoint_y = tempmin_y;

	//minpoint_x_current = tempmin_x;
	//minpoint_y_current = tempmin_y;
	//minpoint_depth_current = tempmin;

	//draw_rect(image, *minpoint_x, *minpoint_y, 3, 16);
	//draw_rect(image_disp, *minpoint_x, *minpoint_y, 3, 16);
	//return ppDepth;

/*
	// image file dump
	stringstream ss;
	ss<<file_num;
	ss<<".xml";
	file_num ++;
	
	cv::FileStorage File(ss.str().c_str(),cv::FileStorage::WRITE);
	cv::Mat b(image);
	cv::write(File,"mat",b);
	File.release();
*/
}


IplImage* perspective_transpose (IplImage *depthCV_src, cv::Point2f pts_src[], cv::Point2f pts_dst[])
{
	
	// convert IplImage* -> cv::Mat
	cv::Mat src_img(depthCV_src); 
	cv::Mat dst_img;

	//cv::Point2f pts_src[] = {cv::Point2f(0,0.),cv::Point2f(0,479.),cv::Point2f(639,479.),cv::Point2f(639,0.)};
	//cv::Point2f pts_dst[] = {cv::Point2f(0,0.),cv::Point2f(0,479.),cv::Point2f(639,379.),cv::Point2f(639,100.)};

	cv::Mat perspective_matrix = cv::getPerspectiveTransform(pts_src, pts_dst);
	cv::warpPerspective(src_img, dst_img, perspective_matrix, src_img.size(), cv::INTER_CUBIC);

	// convert cv::Mat -> IplImage
	IplImage depthCV_dst_temp = dst_img;
	IplImage *depthCV_dst = cvCloneImage(&depthCV_dst_temp);
	return depthCV_dst;
	
}

void camera_run()
{
	//IplImage *Ipl_depth_disp;

	//int crop_value = 220;

	//int tempmid=0;
	
	//minpoint_x=0, minpoint_y=0;

	CvCapture *capture;
		
	XnStatus rc;

	EnumerationErrors errors;

	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
	rc = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);

	// aligning rgb & depth
	g_depth.GetAlternativeViewPointCap().SetViewPoint(g_image);

	//cvNamedWindow("Depth",1);
	//cvNamedWindow("Image",1);

	Ipl_rgb = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8, 3);
	//IplImage* Ipl_depth = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8 , 3);
	Ipl_depth = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),16 , 1);
	//Ipl_depth_disp = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),16 , 1);
/*
	// for another color camera
	capture = cvCreateCameraCapture(0);
	if(capture) {
		cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH,640);   
		cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT,480); 
	}
*/
	ImageMetaData g_imageMD;
	DepthMetaData g_depthMD;

	// for image write to file
	//cv::VideoWriter my_write2("a.wmv", CV_FOURCC('W','M','V','1'),29.97, cv::Size(640,480), 0);
	//cv::Mat mat_depth;
	//IplImage *Ipl_depth_write = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8 , 3);

	// OpenGL test
	//glutInitWindowSize(640,480);
	//glutCreateWindow("OpenGL Test");
	//ReSizeGLScene(640, 480);
	//glutDisplayFunc(MY_OPENGL_TEST);
	//glutDisplayFunc(DrawGLScene);
	//glutMainLoop();
	
	//CreateGLWindow("NeHe's First Polygon Tutorial",640,480,16,fullscreen);
	//glutCreateWindow("OpenGL Test");
	//glutDisplayFunc(DrawGLScene);
	//glutMainLoop();

	// depth capture & conversion for OpenGL 3d plot...
	//g_depth.GetMetaData(g_depthMD);
	//Depth_to_IplImage( Ipl_depth, g_depth, ppDepth, &tempmid, &tempmin, &minpoint_x, &minpoint_y, &g_depthMD );

	//PostureClassifier posture;
	//int hand_palm = 0;

	while(!PROG_EXIT)
	{
		//tempmid=0;

		rc = g_context.WaitAnyUpdateAll();
		if (camera_update)
		{
			// rgb & depth capture
			g_depth.GetMetaData(g_depthMD);
			g_image.GetMetaData(g_imageMD);

			// rgb & depth conversion
			//Depth_to_IplImage( Ipl_depth, Ipl_depth_disp, g_depth, ppDepth, &maxdepth, &tempmid, &minpoint_depth, &minpoint_x, &minpoint_y, &g_depthMD );
			Depth_to_IplImage( Ipl_depth, g_depth, ppDepth, &g_depthMD );
			RGB_to_IplImage(Ipl_rgb, g_image, &g_imageMD);
		
			//get_Depth_minpoint ( Ipl_depth, Ipl_depth_disp, &maxdepth, &tempmid, &minpoint_depth, &minpoint_x, &minpoint_y);
			// crop rgb by depth value
			//crop_rgb_by_depth ( Ipl_rgb, Ipl_depth , crop_value);

			// for another color camera
			//Ipl_rgb2 = cvQueryFrame(capture);
			
			camera_stop = 0;
/*
			if (warp_enable) {
				cv::Point2f warp_pts_src[] = {warp_src_tl, warp_src_bl, warp_src_br, warp_src_tr};
				cv::Point2f warp_pts_dst[] = {warp_dst_tl, warp_dst_bl, warp_dst_br, warp_dst_tr};
				Ipl_depth_warp = perspective_transpose(Ipl_depth, warp_pts_src, warp_pts_dst);
					
			}
*/			
/*			
			// scaling image TEST
			CvPoint2D32f srcTri[3], dstTri[3];
			CvMat *warp_mat = cvCreateMat(2, 3, CV_32FC1); 

			Ipl_rgb2_temp = cvCloneImage(Ipl_rgb2);
			float scale_ratio_h = 1/1.07;
			float scale_ratio_v = 1/2.07;
			
			srcTri[0].x = 0;                          // X1
			srcTri[0].y = 0; 
			srcTri[1].x = Ipl_rgb2->width - 1;    // Y1
			srcTri[1].y = 0; 
			srcTri[2].x = 0;                          // Z1 
			srcTri[2].y = Ipl_rgb2->height - 1;

			dstTri[0].x = 0;                          // X2
			dstTri[0].y = 0; 
			dstTri[1].x = scale_ratio_h * (Ipl_rgb2_temp->width - 1);    // Y2
			dstTri[1].y = 0; 
			dstTri[2].x = 0;                          // Z2
			dstTri[2].y = scale_ratio_v * (Ipl_rgb2_temp->height - 1);

			cvGetAffineTransform(srcTri, dstTri, warp_mat);
			cvWarpAffine( Ipl_rgb2, Ipl_rgb2_temp, warp_mat);//, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS); // 선형보간
*/
		}
		else
		{
			camera_stop = 1;
		}
/*
		CvPoint3D32f body_center;

		if (face_detect_enable)
		{
			//face_detect_return (Ipl_rgb, &face_pos, &face_size);
			//draw_rect (Ipl_rgb, face_pos.x, face_pos.y, face_size, 8);
			draw_rect (Ipl_depth_disp, face_pos.x, face_pos.y, face_size, 16);
			
			body_center.x = face_pos.x;
			if (face_pos.y + 4*face_size >= 480 )
				body_center.y = (480 + face_pos.y) >> 2;
			else
				body_center.y = face_pos.y + 2*face_size;
		}
		else
		{
			body_center.x = 320;
			body_center.y = 240;
			face_size = 30;
		}

        cv::Mat depthMap;
		depthMap = Ipl_depth;
		body_center.z = (float)depthMap.at<short>(body_center.y,body_center.x); // be careful

		// hand palm detection
		//cv::Point low_point(minpoint_x, minpoint_y);
		cv::Point low_point(minpoint_x_current, minpoint_y_current);
		posture.open(depthMap,low_point, 200);
		hand_palm = posture.posture_detection();
*/		
		//minpoint_x =  posture.center_of_hand().x;
		//minpoint_y =  posture.center_of_hand().y;
		
		// for debugging
		//if (mouse_state == MOUSE_RIGHT_PUSH)
		//	draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 8, 16);
		//else if (mouse_state == MOUSE_LEFT_PUSH)
		//	draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 13, 16);
		//else
		//	draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 3, 16);

		//posture.~PostureClassifier();
/*
		if (rel_mouse_con | abs_mouse_con)
		{
			hmi_center(minpoint_x, minpoint_y, minpoint_depth,
					body_center.x, body_center.y, body_center.z, face_size, hand_palm);
		}
*/
//		if ( rel_mouse_con )
//			hmi_center_relative(minpoint_x, minpoint_y, minpoint_depth,
//							body_center.x, body_center.y, body_center.z, face_size, hand_palm);
//		else if (abs_mouse_con )
//			hmi_center_absolute(minpoint_x, minpoint_y, minpoint_depth);

		//cvShowImage("Depth", Ipl_depth );
//		cvShowImage("Depth_disp", Ipl_depth_disp );
//		cvShowImage("Image", Ipl_rgb );
		//cvShowImage("Image2", Ipl_rgb2);
		//cvShowImage("Image2_temp", Ipl_rgb2_temp);
/*
		if (warp_enable) {
			cvShowImage("Depth_warp", Ipl_depth_warp );
		}
*/
		// write depth image to file
		//cvCvtColor(Ipl_depth_disp, Ipl_depth_write, CV_GRAY2BGR);
		//mat_depth = Ipl_rgb;//depth_write;
		//my_write2 << mat_depth;

		//glutDisplayFunc(my_opengl_3d_plot);
		//glutMainLoop();

		//if( cvWaitKey(1) >= NULL ) break;
		
	}
	
	cvReleaseImage( &Ipl_depth );
	//cvReleaseImage( &Ipl_depth_disp );
	cvReleaseImage( &Ipl_rgb );
	//cvReleaseImage( &Ipl_rgb2 );
	//cvReleaseImage( &Ipl_rgb2_temp );
/*	
	cvDestroyWindow("Depth_disp");
	cvDestroyWindow("Depth");
	cvDestroyWindow("Image");
	cvDestroyWindow("Image2");
	cvDestroyWindow("Image2_temp");
*/
	// file write release
	//my_write2.~VideoWriter();

	return;
}


void camera_operate()
{
	IplImage *Ipl_depth_disp, *Ipl_calibration_test;

	//int crop_value = 220;

	//int tempmid=0;
	
	minpoint_x=0, minpoint_y=0;
/*
	CvCapture *capture;
		
	XnStatus rc;

	EnumerationErrors errors;

	rc = g_context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
	rc = g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depth);
	rc = g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_image);

	// aligning rgb & depth
	g_depth.GetAlternativeViewPointCap().SetViewPoint(g_image);

	cvNamedWindow("Depth",1);
	cvNamedWindow("Image",1);

	Ipl_rgb = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8, 3);
	//IplImage* Ipl_depth = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8 , 3);
	Ipl_depth = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),16 , 1);
*/
	Ipl_depth_disp = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),16 , 1);
	Ipl_calibration_test = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y), 8 , 3);

	// for another color camera
//	capture = cvCreateCameraCapture(0);
//	if(capture) {
//		cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH,640);   
//		cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT,480); 
//	}
	
//	ImageMetaData g_imageMD;
//	DepthMetaData g_depthMD;

	// for image write to file
	//cv::VideoWriter my_write2("a.wmv", CV_FOURCC('W','M','V','1'),29.97, cv::Size(640,480), 0);
	//cv::Mat mat_depth;
	//IplImage *Ipl_depth_write = cvCreateImage(cvSize(WIN_SIZE_X, WIN_SIZE_Y),8 , 3);

	// OpenGL test
	//glutInitWindowSize(640,480);
	//glutCreateWindow("OpenGL Test");
	//ReSizeGLScene(640, 480);
	//glutDisplayFunc(MY_OPENGL_TEST);
	//glutDisplayFunc(DrawGLScene);
	//glutMainLoop();
	
	//CreateGLWindow("NeHe's First Polygon Tutorial",640,480,16,fullscreen);
	//glutCreateWindow("OpenGL Test");
	//glutDisplayFunc(DrawGLScene);
	//glutMainLoop();

	// depth capture & conversion for OpenGL 3d plot...
	//g_depth.GetMetaData(g_depthMD);
	//Depth_to_IplImage( Ipl_depth, g_depth, ppDepth, &tempmid, &tempmin, &minpoint_x, &minpoint_y, &g_depthMD );

	PostureClassifier posture;
	int hand_palm = 0;

	while( !depth_updating ) {}

	while(!PROG_EXIT)
	{
		//tempmid=0;

		//rc = g_context.WaitAnyUpdateAll();
	
		while( depth_updating ) {}

		if (camera_update)
		{
			// rgb & depth capture
			//g_depth.GetMetaData(g_depthMD);
			//g_image.GetMetaData(g_imageMD);

			// rgb & depth conversion
			//Depth_to_IplImage( Ipl_depth, Ipl_depth_disp, g_depth, ppDepth, &maxdepth, &tempmid, &minpoint_depth, &minpoint_x, &minpoint_y, &g_depthMD );
			//Depth_to_IplImage( Ipl_depth, g_depth, ppDepth, &g_depthMD );
			//RGB_to_IplImage(Ipl_rgb, g_image, &g_imageMD);
		
			get_Depth_minpoint ( Ipl_depth, Ipl_depth_disp, &maxdepth, &minpoint_depth, &minpoint_x, &minpoint_y);
			// crop rgb by depth value
			//crop_rgb_by_depth ( Ipl_rgb, Ipl_depth , crop_value);

			// for another color camera
			//Ipl_rgb2 = cvQueryFrame(capture);
			
			camera_stop = 0;
/*
			if (warp_enable) {
				cv::Point2f warp_pts_src[] = {warp_src_tl, warp_src_bl, warp_src_br, warp_src_tr};
				cv::Point2f warp_pts_dst[] = {warp_dst_tl, warp_dst_bl, warp_dst_br, warp_dst_tr};
				Ipl_depth_warp = perspective_transpose(Ipl_depth, warp_pts_src, warp_pts_dst);
					
			}
*/			
/*			
			// scaling image TEST
			CvPoint2D32f srcTri[3], dstTri[3];
			CvMat *warp_mat = cvCreateMat(2, 3, CV_32FC1); 

			Ipl_rgb2_temp = cvCloneImage(Ipl_rgb2);
			float scale_ratio_h = 1/1.07;
			float scale_ratio_v = 1/2.07;
			
			srcTri[0].x = 0;                          // X1
			srcTri[0].y = 0; 
			srcTri[1].x = Ipl_rgb2->width - 1;    // Y1
			srcTri[1].y = 0; 
			srcTri[2].x = 0;                          // Z1 
			srcTri[2].y = Ipl_rgb2->height - 1;

			dstTri[0].x = 0;                          // X2
			dstTri[0].y = 0; 
			dstTri[1].x = scale_ratio_h * (Ipl_rgb2_temp->width - 1);    // Y2
			dstTri[1].y = 0; 
			dstTri[2].x = 0;                          // Z2
			dstTri[2].y = scale_ratio_v * (Ipl_rgb2_temp->height - 1);

			cvGetAffineTransform(srcTri, dstTri, warp_mat);
			cvWarpAffine( Ipl_rgb2, Ipl_rgb2_temp, warp_mat);//, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS); // 선형보간
*/
		}
		else
		{
			camera_stop = 1;
		}

		CvPoint3D32f body_center;

		if (face_detect_enable)
		{
			//face_detect_return (Ipl_rgb, &face_pos, &face_size);
			//draw_rect (Ipl_rgb, face_pos.x, face_pos.y, face_size, 8);
			draw_rect (Ipl_depth_disp, face_pos.x, face_pos.y, face_size, 16);
			
			body_center.x = face_pos.x;
			if (face_pos.y + 4*face_size >= 480 )
				body_center.y = (480 + face_pos.y) >> 2;
			else
				body_center.y = face_pos.y + 2*face_size;
		}
		else
		{
			body_center.x = 320;
			body_center.y = 240;
			face_size = 30;
		}

        cv::Mat depthMap;
		depthMap = Ipl_depth;
		body_center.z = (float)depthMap.at<short>(body_center.y,body_center.x); // be careful

		// hand palm detection
		//cv::Point low_point(minpoint_x, minpoint_y);
		cv::Point low_point(minpoint_x_current, minpoint_y_current);
		posture.open(depthMap,low_point, 200);
		hand_palm = posture.posture_detection();
		
		//minpoint_x =  posture.center_of_hand().x;
		//minpoint_y =  posture.center_of_hand().y;
		
		// for debugging
		//if (mouse_state == MOUSE_RIGHT_PUSH)
		if (scrol_state == 1)
			draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 8, 16);
		else if (scrol_state == -1)
			draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 13, 16);
		else
			draw_rect(Ipl_depth_disp, minpoint_x, minpoint_y, 3, 16);

		//posture.~PostureClassifier();

		if (rel_mouse_con | abs_mouse_con)
		{
			hmi_center(minpoint_x, minpoint_y, minpoint_depth,
					body_center.x, body_center.y, body_center.z, face_size, hand_palm);
		}

		// arm horizontal/vertical measure
		if (arm_horizontal_measure | arm_vertical_measure)
		{
			if (arm_horizontal_measure)
				calibration_test(Ipl_depth_disp,Ipl_calibration_test,minpoint_x, minpoint_y, minpoint_depth, 0);
			else if (arm_vertical_measure)
				calibration_test(Ipl_depth_disp,Ipl_calibration_test,minpoint_x, minpoint_y, minpoint_depth, 1);

			cvShowImage("calibration_test", Ipl_calibration_test );
		}

		//cvShowImage("Depth", Ipl_depth );
	
		cvShowImage("Depth_disp", Ipl_depth_disp );
		//cvShowImage("Image", Ipl_rgb );
		
		//cvShowImage("Image2", Ipl_rgb2);
		//cvShowImage("Image2_temp", Ipl_rgb2_temp);
/*
		if (warp_enable) {
			cvShowImage("Depth_warp", Ipl_depth_warp );
		}
*/
		// write depth image to file
		//cvCvtColor(Ipl_depth_disp, Ipl_depth_write, CV_GRAY2BGR);
		//mat_depth = Ipl_rgb;//depth_write;
		//my_write2 << mat_depth;

		//glutDisplayFunc(my_opengl_3d_plot);
		//glutMainLoop();

		if( cvWaitKey(1) >= NULL ) break;
		
	}
	
	//cvReleaseImage( &Ipl_depth );
	cvReleaseImage( &Ipl_depth_disp );
	//cvReleaseImage( &Ipl_rgb );
	//cvReleaseImage( &Ipl_rgb2 );
	//cvReleaseImage( &Ipl_rgb2_temp );
	cvReleaseImage( &Ipl_calibration_test );

	cvDestroyWindow("Depth_disp");
	//cvDestroyWindow("Depth");
	//cvDestroyWindow("Image");
	//cvDestroyWindow("Image2");
	//cvDestroyWindow("Image2_temp");
	cvDestroyWindow("calibration_test");

	// file write release
	//my_write2.~VideoWriter();

	return;
}