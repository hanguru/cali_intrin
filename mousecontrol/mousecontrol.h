#pragma once

#include "resource.h"

#include <WinUser.h> // Required for the ::SendInput function

#include <cv.h>
#include <highgui.h>
#include <XnCppWrapper.h>
#include <math.h>

// OpenGL 관련
#include <gl\glut.h>
#include <gl\GL.h>
#include <gl\GLU.h>

// pthread 관련
#include <pthread.h>

// 전역변수
IplImage *Ipl_rgb, *Ipl_depth, *Ipl_rgb2, *Ipl_depth_warp;

cv::Point2f warp_src_tl, warp_src_bl, warp_src_br, warp_src_tr;
cv::Point2f warp_dst_tl, warp_dst_bl, warp_dst_br, warp_dst_tr;

int camera_update, camera_stop;
int warp_enable;

int minpoint_x, minpoint_y, minpoint_depth, maxdepth;

void draw_rect (IplImage *Ipl_img, int x, int y, int size, int data_bit);
void camera_run();
void camera_operate();
void calibration_step1();
void calibration_step2();
void rgb_test();
void warp_test();

int WIN_SIZE_X = 640;
int WIN_SIZE_Y = 480;

int opengl_transform();

int opengl_exit = 0;

bool PROG_EXIT;

bool rel_mouse_con=0, abs_mouse_con=0;

bool face_detect_enable = 0;

extern void face_detect_run();

CvPoint face_pos;
int face_size;
bool face_pos_valid=0;

bool depth_updating=0, rgb_updating=0, depth_copying=0;

bool mouse_left_hand = true;

bool arm_horizontal_measure=false, arm_vertical_measure=false;