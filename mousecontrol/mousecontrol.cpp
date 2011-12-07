// mousecontrol.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "mousecontrol.h"

#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

pthread_t glutThreadId, glutThreadId2;

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOUSECONTROL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���� �ʱ�ȭ
	camera_update = 0;
	camera_stop = 1;
	warp_enable = 0;
	PROG_EXIT = 0;

	warp_src_tl = cv::Point2f(0,0.);	warp_src_tr = cv::Point2f(639,0.);
	warp_src_bl = cv::Point2f(0,479.);	warp_src_br = cv::Point2f(639,479.);
			
	warp_dst_tl = cv::Point2f(0,0.);	warp_dst_tr = cv::Point2f(639,0.);
	warp_dst_bl = cv::Point2f(0,479.);	warp_dst_br = cv::Point2f(639,479.);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOUSECONTROL));

	// �⺻ �޽��� �����Դϴ�.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
//  ����:
//
//    Windows 95���� �߰��� 'RegisterClassEx' �Լ����� ����
//    �ش� �ڵ尡 Win32 �ý��۰� ȣȯ�ǵ���
//    �Ϸ��� ��쿡�� �� �Լ��� ����մϴ�. �� �Լ��� ȣ���ؾ�
//    �ش� ���� ���α׷��� �����
//    '�ùٸ� ������' ���� �������� ������ �� �ֽ��ϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSECONTROL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MOUSECONTROL);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      //CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	  CW_USEDEFAULT, 0, 600, 400, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


extern void mouse_wheel(int wheel_num);
void MouseMove()
{
/*
	// Toggle Caps Lock key:
    INPUT input[2];
    ::ZeroMemory(input, sizeof(input));        
    input[0].type = input[1].type = INPUT_KEYBOARD;
    input[0].ki.wVk  = input[1].ki.wVk = VK_CAPITAL;        
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // THIS IS IMPORTANT
    ::SendInput(2, input, sizeof(INPUT));
*/
	// Get Screen Resolution
	DEVMODE mode;
	::EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS, &mode);
	
	INPUT in;
	ZeroMemory(&in, sizeof(in));
	in.type = INPUT_MOUSE;
	in.mi.dx = 40 * (65535 / mode.dmPelsWidth);
	in.mi.dy = 20 * (65535 / mode.dmPelsHeight);
 
	//MOUSEEVENTF_MOVE	���콺 �̵�
	//MOUSEEVENTF_LEFTDOWN	���콺 ���� ���� DOWN
	//MOUSEEVENTF_LEFTUP	���콺 ���� ���� UP
	//MOUSEEVENTF_RIGHTDOWN	���콺 ������ ���� DOWN
	//MOUSEEVENTF_RIGHTUP	���콺 ���� ���� UP
	//MOUSEEVENTF_MIDDLEDOWN	���콺 ��� ���� DOWN
	//MOUSEEVENTF_MIDDLEDOUP	���콺 ��� ���� UP
	//MOUSEEVENTF_WHEEL	���콺 �� ������

	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
	//::SendInput(1, &in, sizeof(in));

	// ���콺 Ŀ���� ��ǥ�� pt�� ��� �´�.
	tagPOINT pt;
	GetCursorPos(&pt);
	
	// Move the Mouse position without EVENT : FASTER
	//SetCursorPos(pt.x + 20, pt.y +20);
	SetCursorPos(40, 20);

	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
	//::SendInput(1, &in, sizeof(in));
	//::SendInput(1, &in, sizeof(in));
	in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN| MOUSEEVENTF_RIGHTUP;
	::SendInput(1, &in, sizeof(in));

	SetCursorPos(340, 20);
	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN;
	//::SendInput(1, &in, sizeof(in));

	// wheel test
	SetCursorPos(1000, 800);
	::SendInput(1, &in, sizeof(in));
	mouse_wheel(-3);

}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����: �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �޴� ������ ���� �м��մϴ�.
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_MOUSE_MOVE1:
			MouseMove();
			break;
		case IDM_CAMERA_RUN1:
			camera_update = 1;
			depth_updating = 0;
			rgb_updating = 0;
			pthread_create(&glutThreadId, NULL, (void *(__cdecl *)(void *))camera_run, 0);
			pthread_create(&glutThreadId, NULL, (void *(__cdecl *)(void *))camera_operate, 0);
			break;
		case IDM_RELATIVE_MOUSE_CONTROL1:
			abs_mouse_con = false;
			rel_mouse_con ^= true;
			break;
		case IDM_ABSOLUTE_MOUSE_CONTROL1:
			rel_mouse_con = false;
			abs_mouse_con ^= true;
			break;
		case IDM_FACE_DETECT_ENAGLE_DISABLE1:
			face_detect_enable ^= true;
			if (face_detect_enable)
			{
				pthread_create(&glutThreadId, NULL, (void *(__cdecl *)(void *))face_detect_run, 0);
			}
			
			break;
		case IDM_MOUSE_LEFT_OR_RIGHT_HAND1:
			mouse_left_hand ^= true;
			break;
		case IDM_ARM_HORIZONTAL_MEASURE1:
			arm_horizontal_measure ^= true;
			break;
		case IDM_ARM_VERTICAL_MEASURE1:
			arm_vertical_measure ^= true;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PROG_EXIT = 1;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
