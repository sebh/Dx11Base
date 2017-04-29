
#include "WindowHelper.h"


LRESULT CALLBACK WindowProcess(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}


WindowHelper::WindowHelper(HINSTANCE hInstance, const RECT& clientRect, int nCmdShow, LPCWSTR windowName)
	: mHInstance(hInstance)
	, mNCmdShow(nCmdShow)
	, mClientRect(clientRect)
{
	mInput.init();

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
	mWc.lpfnWndProc = WindowProcess;
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


void WindowHelper::processMouseMessage(MSG& msg)
{
	// TODO WM_NCMOUSELEAVE 

	mInput.mInputStatus.mouseX = LOWORD(msg.lParam);
	mInput.mInputStatus.mouseY = HIWORD(msg.lParam);

	InputEvent event;
	event.mouseX = mInput.mInputStatus.mouseX;
	event.mouseY = mInput.mInputStatus.mouseY;
	switch (msg.message)
	{
	case WM_MOUSEMOVE:
		event.type = etMouseMoved;
		break;
	case WM_LBUTTONDOWN:
		event.type = etMouseButtonDown;
		event.mouseButton = mbLeft;
		break;
	case WM_MBUTTONDOWN:
		event.type = etMouseButtonDown;
		event.mouseButton = mbMiddle;
		break;
	case WM_RBUTTONDOWN:
		event.type = etMouseButtonDown;
		event.mouseButton = mbRight;
		break;
	case WM_LBUTTONUP:
		event.type = etMouseButtonUp;
		event.mouseButton = mbLeft;
		break;
	case WM_MBUTTONUP:
		event.type = etMouseButtonUp;
		event.mouseButton = mbMiddle;
		break;
	case WM_RBUTTONUP:
		event.type = etMouseButtonUp;
		event.mouseButton = mbRight;
		break;
	case WM_LBUTTONDBLCLK:
		event.type = etMouseButtonDoubleClick;
		event.mouseButton = mbLeft;
		break;
	case WM_MBUTTONDBLCLK:
		event.type = etMouseButtonDoubleClick;
		event.mouseButton = mbMiddle;
		break;
	case WM_RBUTTONDBLCLK:
		event.type = etMouseButtonDoubleClick;
		event.mouseButton = mbRight;
		break;

	// TODO
	//case WM_MOUSEWHEEL:
	//case WM_MOUSEHWHEEL:
	}
	mInput.mInputEvents.push_back(event);
}


void WindowHelper::processKeyMessage(MSG& msg)
{
}


bool WindowHelper::processSingleMessage(MSG& msg)
{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// translate keystroke messages into the right format
		TranslateMessage(&msg);

		// send the message to the WindowProc function
		DispatchMessage(&msg);

		switch (msg.message)
		{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_NCMOUSELEAVE:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
			processMouseMessage(msg);
			break;
		}

		// Message processed
		return true;
	}

	// No mesagge processed
	return false;
}
