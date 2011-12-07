// mousecontrol.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "mousecontrol.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

pthread_t glutThreadId, glutThreadId2;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
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

 	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOUSECONTROL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 전역 변수 초기화
	camera_update = 0;
	camera_stop = 1;
	warp_enable = 0;
	PROG_EXIT = 0;

	warp_src_tl = cv::Point2f(0,0.);	warp_src_tr = cv::Point2f(639,0.);
	warp_src_bl = cv::Point2f(0,479.);	warp_src_br = cv::Point2f(639,479.);
			
	warp_dst_tl = cv::Point2f(0,0.);	warp_dst_tr = cv::Point2f(639,0.);
	warp_dst_bl = cv::Point2f(0,479.);	warp_dst_br = cv::Point2f(639,479.);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOUSECONTROL));

	// 기본 메시지 루프입니다.
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
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
//  설명:
//
//    Windows 95에서 추가된 'RegisterClassEx' 함수보다 먼저
//    해당 코드가 Win32 시스템과 호환되도록
//    하려는 경우에만 이 함수를 사용합니다. 이 함수를 호출해야
//    해당 응용 프로그램에 연결된
//    '올바른 형식의' 작은 아이콘을 가져올 수 있습니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
 
	//MOUSEEVENTF_MOVE	마우스 이동
	//MOUSEEVENTF_LEFTDOWN	마우스 왼쪽 버턴 DOWN
	//MOUSEEVENTF_LEFTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_RIGHTDOWN	마우스 오른쪽 버턴 DOWN
	//MOUSEEVENTF_RIGHTUP	마우스 왼쪽 버턴 UP
	//MOUSEEVENTF_MIDDLEDOWN	마우스 가운데 버턴 DOWN
	//MOUSEEVENTF_MIDDLEDOUP	마우스 가운데 버턴 UP
	//MOUSEEVENTF_WHEEL	마우스 휠 움직임

	//in.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
	//::SendInput(1, &in, sizeof(in));

	// 마우스 커서의 좌표를 pt로 얻어 온다.
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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
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
		// 메뉴 선택을 구문 분석합니다.
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
		// TODO: 여기에 그리기 코드를 추가합니다.
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

// 정보 대화 상자의 메시지 처리기입니다.
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
