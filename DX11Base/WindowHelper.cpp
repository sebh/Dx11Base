
#include "WindowHelper.h"

WindowHelper::WindowHelper(HINSTANCE hInstance, const RECT& clientRect, int nCmdShow, WNDPROC winProcFun, LPCWSTR windowName)
	: mHInstance(hInstance)
	, mNCmdShow(nCmdShow)
	, mClientRect(clientRect)
{
	// And create the rectangle that will allow it
	RECT rect = { 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top }; // set the size, but not the position otherwise does not seem to work
	DWORD style = (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX); // WS_OVERLAPPEDWINDOW, add simple style as paramter
	BOOL menu = false;
	AdjustWindowRect(&rect, style, menu);
	//Get the required window dimensions
	int WindowWidth = rect.right - rect.left; //Required width
	//WindowWidth += (2 * GetSystemMetrics(SM_CXFIXEDFRAME)); //Add frame widths
	int WindowHeight = rect.bottom - rect.top; //Required height
	//WindowHeight += GetSystemMetrics(SM_CYCAPTION); //Titlebar height
	//WindowHeight += GetSystemMetrics(SM_CYMENU); //Uncomment for menu bar height
	//WindowHeight += (2 * GetSystemMetrics(SM_CYFIXEDFRAME)); //Frame heights

	// clear out the window class for use
	ZeroMemory(&mWc, sizeof(WNDCLASSEX));
	// fill in the struct with the needed information
	mWc.cbSize = sizeof(WNDCLASSEX);
	mWc.style = CS_HREDRAW | CS_VREDRAW;
	mWc.lpfnWndProc = winProcFun;
	mWc.hInstance = mHInstance;
	mWc.hCursor = LoadCursor(NULL, IDC_ARROW);
	mWc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	mWc.lpszClassName = L"WindowClass1";

	// register the window class
	RegisterClassEx(&mWc);

	// create the window and use the result as the handle
	mHWnd = CreateWindowEx(NULL,
		L"WindowClass1",					// name of the window class
		windowName,							// title of the window
		style,								// not resizable
		clientRect.top,						// x-position of the window
		clientRect.left,					// y-position of the window
		WindowWidth,						// width of the window
		WindowHeight,						// height of the window
		NULL,								// we have no parent window, NULL
		NULL,								// we aren't using menus, NULL
		hInstance,							// application handle
		NULL);								// used with multiple windows, NULL
}

WindowHelper::~WindowHelper()
{
}


void WindowHelper::showWindow()
{
	ShowWindow(mHWnd, mNCmdShow);
}


bool WindowHelper::processSingleMessage(MSG& msg)
{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessage(&msg);

		// Message processed
		return true;
	}

	// No mesagge processed
	return false;
}
