
// include the basic windows header file
#include "Dx11Base/WindowHelper.h"
#include "Dx11Base/Dx11Device.h"
#include "Game.h"


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

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// Get a window size that matches the desired client size
	const unsigned int desiredPosX = 20;
	const unsigned int desiredPosY = 20;
	const unsigned int desiredClientWidth  = 1280;
	const unsigned int desiredClientHeight = 720;
	RECT clientRect;
	clientRect.left		= desiredPosX;
	clientRect.right	= desiredPosX + desiredClientWidth;
	clientRect.bottom	= desiredPosY + desiredClientHeight;
	clientRect.top		= desiredPosY;

	// Create the window
	WindowHelper win(hInstance, clientRect, nCmdShow, &WindowProcess, L"D3D11 Application");
	win.showWindow();

	// Create the d3d device
	Dx11Device::initialise(win.getHwnd());
	DxGpuPerformance::initialise();

	// Create the game
	Game game;
	game.initialise();

	MSG msg = { 0 };
	while (true)
	{
		if (win.processSingleMessage(msg))
		{
			// a message has been processed

			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
			{
				// process escape key
				break;
			}

			if (msg.message == WM_QUIT)
				break; // time to quit
		}
		else
		{
			const char* frameGpuTimerName = "Frame";
			DxGpuPerformance::startGpuTimer(frameGpuTimerName);
			DxGpuPerformance::startFrame();

			// Game update
			game.update();

			// Game render
			game.render();

			// Swap the back buffer
			g_dx11Device->swap(true);

			DxGpuPerformance::endGpuTimer(frameGpuTimerName);
			DxGpuPerformance::debugPrintTimer();
			DxGpuPerformance::endFrame();
		}
	}

	game.shutdown();
	DxGpuPerformance::shutdown();
	Dx11Device::shutdown();

	// End of application
	return (int)msg.wParam;
}


