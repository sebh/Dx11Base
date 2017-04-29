#pragma once

#include <windows.h>
#include <windowsx.h>
#include "WindowInput.h"

class WindowHelper
{
public:
	WindowHelper(
		HINSTANCE hInstance, 
		const RECT& clientRect, 
		int nCmdShow,
		LPCWSTR windowName);
	~WindowHelper();

	void showWindow();

	bool processSingleMessage(MSG& msg);

	const WindowInputData& getInputData()
	{
		return mInput;
	}
	void clearInputEvents()
	{
		mInput.mInputEvents.clear();
	}

	const HWND getHwnd() { return mHWnd; }

private:
	WindowHelper();

	void processMouseMessage(MSG& msg);
	void processKeyMessage(MSG& msg);

	HINSTANCE		mHInstance;			/// The application instance
	HWND			mHWnd;				/// The handle for the window, filled by a function
	WNDCLASSEX		mWc;				/// This struct holds information for the window class
	RECT			mClientRect;		/// The client rectangle where we render into
	int				mNCmdShow;			/// Window show cmd

	WindowInputData mInput;				/// input event and status (mouse, keyboard, etc.)
};


