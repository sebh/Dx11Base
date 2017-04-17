
#include "Dx11Base/WindowHelper.h"
#include "Dx11Base/Dx11Device.h"

#include "Game.h"

#include <imgui.h>
#include "imgui\imgui_impl_dx11.h"


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

	// Initialise imgui
	ImGui_ImplDX11_Init(win.getHwnd(), g_dx11Device->getDevice(), g_dx11Device->getDeviceContext());

	// Create the game
	Game game;
	game.initialise();

	MSG msg = { 0 };
	while (true)
	{
		if (win.processSingleMessage(msg))
		{
			// A message has been processed

			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
				break;// process escape key

			if (msg.message == WM_QUIT)
				break; // time to quit

			// Update imgui
			ImGui_ImplDX11_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);

			// Take into account window resize
			if (msg.message == WM_SIZE && g_dx11Device != NULL && msg.wParam != SIZE_MINIMIZED)
			{
				ImGui_ImplDX11_InvalidateDeviceObjects();
				// clean up data;		// TODO
				g_dx11Device->getSwapChain()->ResizeBuffers(0, (UINT)LOWORD(msg.lParam), (UINT)HIWORD(msg.lParam), DXGI_FORMAT_UNKNOWN, 0);
				// re create data();	// TODO
				ImGui_ImplDX11_CreateDeviceObjects();
			}
		}
		else
		{
			const char* frameGpuTimerName = "Frame";
			DxGpuPerformance::startGpuTimer(frameGpuTimerName);
			DxGpuPerformance::startFrame();

			ImGui_ImplDX11_NewFrame();

			// Game update
			game.update();

			// Game render
			game.render();

			// Render UI
			{
				// TEST IMGUI FOR NOW
				static ImVec4 clear_col = ImColor(114, 144, 154);
				// 1. Show a simple window
				// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
				{
					static float f = 0.0f;
					ImGui::Text("Hello, world!");
					ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
					ImGui::ColorEdit3("clear color", (float*)&clear_col);
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				}
				// 2. Show another simple window, this time using an explicit Begin/End pair
				{
					ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
					ImGui::Begin("Another Window");
					ImGui::Text("Hello");
					ImGui::End();
				}
				// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
				{
					ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
					ImGui::ShowTestWindow();
				}

				GPU_BEGIN_EVENT("Imgui");
				GPU_SCOPED_TIMER(Imgui);
				ImGui::Render();
				GPU_END_EVENT();
			}

			// Swap the back buffer
			g_dx11Device->swap(true);

			DxGpuPerformance::endGpuTimer(frameGpuTimerName);
			DxGpuPerformance::debugPrintTimer();
			DxGpuPerformance::endFrame();
		}
	}

	game.shutdown();
	ImGui_ImplDX11_Shutdown();
	DxGpuPerformance::shutdown();
	Dx11Device::shutdown();

	// End of application
	return (int)msg.wParam;
}


