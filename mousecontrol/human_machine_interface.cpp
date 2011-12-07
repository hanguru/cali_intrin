
#include "stdafx.h"
#include <WinUser.h> // Required for the ::SendInput function

#include <math.h>

#define MOUSE_IDLE 0
#define MOUSE_RIGHT_PUSH 1
#define MOUSE_LEFT_PUSH 2
#define MOUSE_CLICK_N_MOVE 3
#define MOUSE_READY 4
#define MOUSE_RIGHT_POP 5
#define MOUSE_LEFT_POP 6
#define MOUSE_DRAG 7

int mouse_state = MOUSE_IDLE, mouse_dragging = 0;
int pos_x_ave_3=0, pos_x_ave_5=0, pos_x_ave_10=0;
int pos_y_ave_3=0, pos_y_ave_5=0, pos_y_ave_10=0;
int depth_ave_3=0, depth_ave_5=0, depth_ave_10=0, depth_ave_20=0;
int click_depth, click_x, click_y, state_count;

#define DEPTH_N 20
int x_pos_array[DEPTH_N] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int y_pos_array[DEPTH_N] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int depth_array[DEPTH_N] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int hand_array[DEPTH_N] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int circle_x_up, circle_y_up, circle_speed_up, wheel_valid_up;
int circle_x_dn, circle_y_dn, circle_speed_dn, wheel_valid_dn;
int circle_dir_up = 0;
int scrol_state = 0, scrol_count = 0;

extern bool abs_mouse_con, rel_mouse_con;

extern bool mouse_left_hand;

void mouse_move_rel(int x_move, int y_move)
{
	// 마우스 커서의 좌표를 pt로 얻어 온다.
	tagPOINT pt;
	GetCursorPos(&pt);
	
	// Move the Mouse position without EVENT : FASTER
	SetCursorPos(pt.x + x_move, pt.y + y_move);
}

void mouse_move_abs(int x_pos, int y_pos)
{
	// 마우스 커서의 좌표를 pt로 얻어 온다.
	tagPOINT pt;
	GetCursorPos(&pt);
	
	pt.x = x_pos;
	pt.y = y_pos;

	// Move the Mouse position without EVENT : FASTER
	SetCursorPos(pt.x, pt.y);
}

void mouse_click(int click_type)
{
	INPUT in;
	ZeroMemory(&in, sizeof(in));
	in.type = INPUT_MOUSE;
	
	//MOUSEEVENTF_MOVE	마우스 이동
	//MOUSEEVENTF_LEFTDOWN	마우스 왼쪽 버턴 DOWN
	//MOUSEEVENTF_LEFTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_RIGHTDOWN	마우스 오른쪽 버턴 DOWN
	//MOUSEEVENTF_RIGHTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_MIDDLEDOWN	마우스 가운데 버턴 DOWN
	//MOUSEEVENTF_MIDDLEDOUP	마우스 가운데 버턴 UP
	//MOUSEEVENTF_WHEEL	마우스 휠 움직임
	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;

	if (mouse_left_hand)
	{
		if ( click_type == MOUSE_RIGHT_PUSH )
			in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		else if ( click_type == MOUSE_RIGHT_POP )
			in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		else if ( click_type == MOUSE_LEFT_PUSH )
			in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		else if ( click_type == MOUSE_LEFT_POP )
			in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	}
	else
	{
		if ( click_type == MOUSE_RIGHT_PUSH )
			in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		else if ( click_type == MOUSE_RIGHT_POP )
			in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		else if ( click_type == MOUSE_LEFT_PUSH )
			in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		else if ( click_type == MOUSE_LEFT_POP )
			in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	}
	::SendInput(1, &in, sizeof(in));
}

void mouse_wheel(int wheel_num)
{
	INPUT in;
	ZeroMemory(&in, sizeof(in));
	in.type = INPUT_MOUSE;
	
	//MOUSEEVENTF_MOVE	마우스 이동
	//MOUSEEVENTF_LEFTDOWN	마우스 왼쪽 버턴 DOWN
	//MOUSEEVENTF_LEFTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_RIGHTDOWN	마우스 오른쪽 버턴 DOWN
	//MOUSEEVENTF_RIGHTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_MIDDLEDOWN	마우스 가운데 버턴 DOWN
	//MOUSEEVENTF_MIDDLEDOUP	마우스 가운데 버턴 UP
	//MOUSEEVENTF_WHEEL	마우스 휠 움직임
	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;

	in.mi.dwFlags = MOUSEEVENTF_WHEEL;
	in.mi.mouseData = wheel_num * 120;

	::SendInput(1, &in, sizeof(in));
}

int median_compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}
int median (int depth_array[], int num_of_array)
{
	int n;
	qsort (depth_array, num_of_array, sizeof(int), median_compare);
	return depth_array[num_of_array/2];
}

int scrol_check()
{
	int circle_y    = y_pos_array[2] + y_pos_array[1] + y_pos_array[0];
	int circle_y_d1 = y_pos_array[3] + y_pos_array[2] + y_pos_array[1];

	int x_margin = 100;//50;
	int y_margin = 50;//200;
	int speed_margin = 10;//10;
	int scrol_margin = 30;
	
	if (scrol_count > scrol_margin)
	{
		circle_dir_up = 0;
		circle_x_up = 0;
		circle_y_up = 0;
		circle_speed_up = 0;

		circle_x_dn = 0;
		circle_y_dn = 0;
		circle_speed_dn = 0;
		
		scrol_count = 0;
	}
	else if ( circle_y_d1 <= circle_y )
	{
		if ( circle_dir_up == 1 )
		{
			circle_dir_up = 0;
			circle_x_up = x_pos_array[1];
			circle_y_up = y_pos_array[1];
			circle_speed_up = x_pos_array[0] - x_pos_array[1];
			
			scrol_count = 0;

			if ( ( abs(circle_x_up - circle_x_dn) < x_margin ) &
				 ( abs(circle_y_up - circle_y_dn) > y_margin ) &
				 ( abs(circle_speed_up) > speed_margin ) )
			{
				if ( circle_speed_up > 0 )
				{
					wheel_valid_up = 1;
					if ( wheel_valid_dn == -1 )
					{
						return 1;
					}
				}
				else
				{
					wheel_valid_up = -1;
					if ( wheel_valid_dn == 1 )
					{
						return -1;
					}
				}
			}
			else
			{
				wheel_valid_up = 0;
			}
		}
		else
		{
			scrol_count ++;
		}
	}
	else //if ( circle_y_d1 > circle_y )
	{
		if ( circle_dir_up == 0 )
		{
			circle_dir_up = 1;
			circle_x_dn = x_pos_array[1];
			circle_y_dn = y_pos_array[1];
			circle_speed_dn = x_pos_array[0] - x_pos_array[1];

			scrol_count = 0;

			if ( ( abs(circle_x_up - circle_x_dn) < x_margin ) &
				 ( abs(circle_y_up - circle_y_dn) > y_margin ) &
				 ( abs(circle_speed_dn) > speed_margin ) )
			{
				if ( circle_speed_dn > 0 )
				{
					wheel_valid_dn = 1;
					if ( wheel_valid_up == -1 )
					{
						return -1;
					}
				}
				else
				{
					wheel_valid_dn = -1;
					if ( wheel_valid_up == 1 )
					{
						return 1;
					}
				}
			}
			else
			{
				wheel_valid_dn = 0;
			}
		}
		else
		{
			scrol_count ++;
		}
	}

	return 0;
}

void hmi_center (int x_pos, int y_pos, int z_pos, 
		float body_pos_x, float body_pos_y, float body_float_z, int face_size, int hand_palm)
{
	// position check
	//int face_size = 38;
	//int face_pos_x = 320;
	//int face_pos_y = 120;
	int center_x = body_pos_x;
	int center_y = body_pos_y;
	int center_z = 1000;
	int center_size_x_2 = face_size;//80;
	int center_size_y_2 = face_size;//80;
	int center_size_z_2 = 100;
	int center_size_z_max = 2000; // ignore objects over 2m
	int abs_window_x_2 = face_size*4;
	int abs_window_y_2 = face_size*4;
	int depth_step = 100;
	int click_time = 10;

	int move_step_x=0, move_step_y=0;

	int move_x=0, move_y=0;

	int x_pos_array_tmp[DEPTH_N];
	int y_pos_array_tmp[DEPTH_N];
	int depth_array_tmp[DEPTH_N];
	int hand_array_tmp[DEPTH_N];

	for (int i=DEPTH_N-1; i>0; i--)
	{
		hand_array[i] = hand_array[i-1];
	}
	hand_array[0] = hand_palm;
	for (int i=DEPTH_N-1; i>=0; i--)
	{
		hand_array_tmp[i] = hand_array[i];
	}

	// initialization of hand position
	int hand_ave_5 = median(hand_array_tmp, 5);
	if ( hand_ave_5 ==1)
	{
		for (int i=DEPTH_N-1; i>0; i--)
		{
			x_pos_array[i] = x_pos;
			y_pos_array[i] = y_pos;
			depth_array[i] = z_pos;
			//hand_array[i] = hand_palm;
		}
		for (int i=DEPTH_N-1; i>=0; i--)
		{
			x_pos_array_tmp[i] = x_pos_array[i];
			y_pos_array_tmp[i] = y_pos_array[i];
			depth_array_tmp[i] = depth_array[i];
			//hand_array_tmp[i] = hand_array[i];
		}

		mouse_state = MOUSE_IDLE;
		DEVMODE mode;
		::EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &mode);

		int init_x = mode.dmPelsWidth/2;
		int init_y = mode.dmPelsHeight/2;
		mouse_move_abs(init_x, init_y);
		mouse_click(MOUSE_LEFT_POP);
		mouse_click(MOUSE_RIGHT_POP);
	}
	else 
	{

		for (int i=DEPTH_N-1; i>0; i--)
		{
			x_pos_array[i] = x_pos_array[i-1];
			y_pos_array[i] = y_pos_array[i-1];
			depth_array[i] = depth_array[i-1];
			//hand_array[i] = hand_array[i-1];
		}
		x_pos_array[0] = x_pos;
		y_pos_array[0] = y_pos;
		depth_array[0] = z_pos;
		//hand_array[0] = hand_palm;
		for (int i=DEPTH_N-1; i>=0; i--)
		{
			x_pos_array_tmp[i] = x_pos_array[i];
			y_pos_array_tmp[i] = y_pos_array[i];
			depth_array_tmp[i] = depth_array[i];
			//hand_array_tmp[i] = hand_array[i];
		}
	}

	pos_x_ave_3 = median(x_pos_array_tmp, 3);
	pos_x_ave_5 = median(x_pos_array_tmp, 5);
	pos_x_ave_10 = median(x_pos_array_tmp, 10);
	pos_y_ave_3 = median(y_pos_array_tmp, 3);
	pos_y_ave_5 = median(y_pos_array_tmp, 5);
	pos_y_ave_10 = median(y_pos_array_tmp, 10);

	if (abs_mouse_con)
	{
		x_pos = pos_x_ave_10;
		y_pos = pos_y_ave_10;
	}
	else
	{
		x_pos = pos_x_ave_5;
		y_pos = pos_y_ave_5;
	}

	depth_ave_3 = median(depth_array_tmp, 3);
	depth_ave_5 = median(depth_array_tmp, 5);
	depth_ave_10 = median(depth_array_tmp, 10);
	depth_ave_20 = median(depth_array_tmp, 20);

	//pos_x_ave_3  = 0.7*pos_x_ave_3  + 0.3 * x_pos;
	//pos_x_ave_5  = 0.8*pos_x_ave_5  + 0.2 * x_pos;
	//pos_x_ave_10 = 0.9*pos_x_ave_10 + 0.1 * x_pos;
	//pos_y_ave_3  = 0.7*pos_y_ave_3  + 0.3 * y_pos;
	//pos_y_ave_5  = 0.8*pos_y_ave_5  + 0.2 * y_pos;
	//pos_y_ave_10 = 0.9*pos_y_ave_10 + 0.1 * y_pos;

	//depth_ave_3 = 0.7*depth_ave_3 + 0.3*z_pos;
	//depth_ave_5 = 0.8*depth_ave_5 + 0.2*z_pos;
	//depth_ave_10 = 0.9*depth_ave_10 + 0.1*z_pos;
	//depth_ave_20 = 0.95*depth_ave_20 + 0.05*z_pos;

	double center_distance = sqrt((double)((x_pos - center_x) * (x_pos - center_x) + (y_pos - center_y) * (y_pos - center_y)) );

	if (center_distance == 0)
		center_distance = 0;

	int center_x_pos = (x_pos -center_x)* center_size_x_2 / center_distance; 
	int center_y_pos = (y_pos -center_y)* center_size_x_2 / center_distance; 

	if ( center_distance <  center_size_x_2 )
	{
		// no mouse move
		move_step_x = 0;
	}
	else 
	{
		move_step_x = center_x + center_x_pos - x_pos;
	}

	if ( center_distance <  center_size_x_2 )
	{
		// no mouse move
		move_step_y = 0;
	}
	else 
	{
		if (center_y > 0 )
		{
			center_y = center_y;
		}

		move_step_y = y_pos - center_y - center_y_pos;
	}

	if ( move_step_x > 200)
	{
		move_step_x = move_step_x;
	}
	
	// Scrol state update
	scrol_state = scrol_check();

	// Mouse Click State Machine
	if ( mouse_state == MOUSE_IDLE )
	{
		if ( (depth_ave_3 > depth_ave_5 - depth_step/4) & (depth_ave_3 < depth_ave_5 + depth_step/4) &
			(move_x == 0) & (move_y == 0) )
		{
			//state_count ++;
			//if ( state_count > click_time)
			//{
				state_count = 0;
				mouse_state = MOUSE_READY;
			//}
		}
		//else 
		//{
		//	state_count = 0;
		//}
	}
	else if ( mouse_state == MOUSE_READY )
	{
		//if ((move_step_x == 0) & (move_step_y == 0))
		{
			//if ( hand_ave_5 == 1)
			//{
			//	mouse_state = MOUSE_DRAG;
			//	mouse_click(MOUSE_RIGHT_PUSH);
			//}
			//else 
				if (depth_ave_3 < depth_ave_5 - depth_step)
			{
				mouse_state = MOUSE_RIGHT_PUSH;
				state_count = 0;
				click_depth = depth_ave_5;
				click_x = pos_x_ave_10;
				click_y = pos_y_ave_10;
				mouse_dragging = 0;
			}
			else if (depth_ave_3 > depth_ave_5 + depth_step)
			{
				mouse_state = MOUSE_LEFT_PUSH;
				state_count = 0;
				click_depth = depth_ave_5;
				click_x = pos_x_ave_10;
				click_y = pos_y_ave_10;
				mouse_dragging = 0;
			}
		}
	}
	else if ( mouse_state == MOUSE_RIGHT_PUSH )
	{
		//if ((move_step_x == 0) & (move_step_y == 0))
		{
			if ( (depth_ave_3 > click_depth - depth_step/2) ) /* & 
				(pos_x_ave_3 < click_x + center_size_x_2/4) &
				(pos_x_ave_3 > click_x - center_size_x_2/4) &
				(pos_y_ave_3 < click_y + center_size_y_2/4) &
				(pos_y_ave_3 > click_y - center_size_y_2/4) )
*/			{
				mouse_state = MOUSE_IDLE;
				mouse_click(MOUSE_RIGHT_PUSH);
				mouse_click(MOUSE_RIGHT_POP);
			}
			else if ( (state_count > click_time) ) /* | 
				((pos_x_ave_3 > click_x + center_size_x_2/4) |
				 (pos_x_ave_3 < click_x - center_size_x_2/4) |
				 (pos_y_ave_3 > click_y + center_size_y_2/4) |
				 (pos_y_ave_3 < click_y - center_size_y_2/4)) )
*/			{
				state_count = 0;
				
				//if (mouse_dragging == 2)
				if ( (depth_ave_3 < click_depth + depth_step*2) ) 
				{
					mouse_dragging = 1;
					mouse_click(MOUSE_RIGHT_PUSH);
				}
				else
				{
					mouse_state = MOUSE_IDLE;
				}
			}
			else if (mouse_dragging != 1)
			{
				//if ((move_step_x != 0) | (move_step_y != 0) | mouse_dragging)
				//{
				//	mouse_dragging = 1;
				//	state_count = 0;
				//}
				//else
				{
					//if ((move_step_x != 0) | (move_step_y != 0))
					//if (	(x_pos > click_x + center_size_x_2*2) |
					//		(x_pos < click_x - center_size_x_2*2) |
					//		(y_pos > click_y + center_size_y_2*2) |
					//		(y_pos < click_y - center_size_y_2*2) )
					//{
					//	mouse_dragging = 2;
					//}
					state_count ++;
				}
			}
		}
		//else 
		//{
		//	mouse_state = MOUSE_IDLE;
		//}
	}
	else if ( mouse_state == MOUSE_LEFT_PUSH )
	{
		//if ((move_step_x == 0) & (move_step_y == 0))
		{
			if ( (depth_ave_3 < click_depth + depth_step/2) ) /*&
				(pos_x_ave_3 < click_x + center_size_x_2/4) &
				(pos_x_ave_3 > click_x - center_size_x_2/4) &
				(pos_y_ave_3 < click_y + center_size_y_2/4) &
				(pos_y_ave_3 > click_y - center_size_y_2/4) )
*/			{
				mouse_state = MOUSE_IDLE;
				mouse_click(MOUSE_LEFT_PUSH);
				mouse_click(MOUSE_LEFT_POP);
			}
			else if ( (state_count > click_time) ) /*|
				((pos_x_ave_3 > click_x + center_size_x_2/4) |
				 (pos_x_ave_3 < click_x - center_size_x_2/4) |
				 (pos_y_ave_3 > click_y + center_size_y_2/4) |
				 (pos_y_ave_3 < click_y - center_size_y_2/4)) )
*/
			{
				state_count = 0;
				if (mouse_dragging == 2)
				{
					mouse_state = MOUSE_IDLE;
				}
				else
				{
					mouse_dragging = 1;
					mouse_click(MOUSE_LEFT_PUSH);
				}
			}
			else if (mouse_dragging != 1)
			{
				//if ((move_step_x != 0) | (move_step_y != 0) | mouse_dragging)
				//{
				//	mouse_dragging = 1;
				//	mouse_click(MOUSE_LEFT_PUSH);
				//	state_count = 0;
				//}
				//else
				{
					//if ((move_step_x != 0) | (move_step_y != 0))
					if (	(x_pos > click_x + center_size_x_2*2) |
							(x_pos < click_x - center_size_x_2*2) |
							(y_pos > click_y + center_size_y_2*2) |
							(y_pos < click_y - center_size_y_2*2) )
					{
						mouse_dragging = 2;
					}
					state_count ++;
				}
			}
		}
		//else 
		//{
		//	mouse_state = MOUSE_IDLE;
		//}
	}
	else if ( mouse_state == MOUSE_DRAG )
	{
		if ( hand_ave_5 != 1)
		{
			mouse_state = MOUSE_IDLE;
			mouse_click(MOUSE_RIGHT_POP);
		}
	}

	// Mouse Move
	if (z_pos < center_size_z_max)
	{
		if (rel_mouse_con)
		{
			// mouse wheel
			mouse_wheel(scrol_state);

			// for relative position
			move_x = move_step_x < 0 ? -(move_step_x>>4)*(move_step_x>>4) : (move_step_x>>4)*(move_step_x>>4);
			move_y = move_step_y < 0 ? -(move_step_y>>4)*(move_step_y>>4) : (move_step_y>>4)*(move_step_y>>4);
			mouse_move_rel(move_x, move_y );
		}
		if (abs_mouse_con)
		{
			// for absolute position
			if (x_pos < (center_x - abs_window_x_2))
				move_x = 2*abs_window_x_2 -1;
			else if (x_pos >= (center_x + abs_window_x_2))
				move_x = 0;
			else 
				move_x = (center_x + abs_window_x_2) - x_pos;

			if (pos_y_ave_10 < (center_y - abs_window_y_2))
				move_y = 0;
			else if (y_pos >= (center_y + abs_window_y_2))
				move_y = 2*abs_window_y_2 -1;
			else 
				move_y = y_pos - (center_y - abs_window_y_2);

			// get windows size
			DEVMODE mode;
			::EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &mode);

			move_x = move_x * (mode.dmPelsWidth -1)/ (abs_window_x_2 * 2 -1);
			move_y = move_y * (mode.dmPelsHeight -1)/ (abs_window_y_2 * 2 -1);
			mouse_move_abs(move_x, move_y );

		}
	}
}

