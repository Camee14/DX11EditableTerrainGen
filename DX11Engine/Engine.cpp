#include <iostream>
#include <chrono>

#include "Engine.h"

Engine::Engine() {
	m_screen_width = 0;
	m_screen_height = 0;
}
Engine::Engine(const Engine& other) {

}
Engine::~Engine() {

}
bool Engine::init() {

	bool res;

	initWindows(m_screen_width, m_screen_height);
	initConsole();

	activateWindows();

	im = new InputManager;
	if (!im) {
		return false;
	}
	res = im->init(m_screen_width, m_screen_height, hwnd, hinstance);
	if (!res) {
		return false;
	}

	/*em = new EntityManager;
	if (!em) {
		return false;
	}*/

	renderer = new Renderer;
	if (!renderer) {
		return false;
	}
	res = renderer->init(m_screen_width, m_screen_height, hwnd);
	if (!res) {
		return false;
	}

	res = gameStart();
	if (!res) {
		return false;
	}
	return true;
}
void Engine::run() {
	MSG msg;
	bool done = false, res;

	auto start_time = std::chrono::high_resolution_clock::now();
	auto end_time = std::chrono::high_resolution_clock::now();

	ZeroMemory(&msg, sizeof(MSG));

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			done = true;
		}else {
			start_time = std::chrono::high_resolution_clock::now();
			res = update();
			end_time = std::chrono::high_resolution_clock::now();
			if (!res) {
				done = true;
			}
			delta_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1000000.0f;
		}
	}

}
bool Engine::update() {
	bool res;
	
	if (im->onKeyDown(DIK_ESCAPE)) {
		return false;
	}

	res = gameUpdate(delta_time);
	if (!res) {
		return false;
	}

	//em->updateEntities(delta_time);

	res = gameDraw();
	if (!res) {
		return false;
	}

	res = im->update();
	if (!res) {
		return false;
	}

	return true;
}
void Engine::initWindows(int& screen_width, int& screen_height) {
	WNDCLASSEX wc;
	DEVMODE screen_settings;
	int posX, posY;

	ApplicationHandle = this;

	hinstance = GetModuleHandle(NULL);

	app_name = "DX11Engine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = app_name;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	screen_width = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);

	int window_settings = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	if (FULL_SCREEN) {
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&screen_settings, 0, sizeof(screen_settings));
		screen_settings.dmSize = sizeof(screen_settings);
		screen_settings.dmPelsWidth = (unsigned long)screen_width;
		screen_settings.dmPelsHeight = (unsigned long)screen_height;
		screen_settings.dmBitsPerPel = 32;
		screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}else{
		screen_width = 1600;
		screen_height = 900;

		window_settings = window_settings | WS_OVERLAPPEDWINDOW;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screen_width) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	}
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW, 
		app_name, 
		app_name,
		window_settings,
		posX, 
		posY, 
		screen_width, 
		screen_height, 
		NULL, 
		NULL, 
		hinstance, 
		NULL
	);

	/*ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	ShowCursor(false);*/

}
void Engine::initConsole() {
	if (!AllocConsole()) {
		MessageBox(NULL, "The console window was not created", NULL, MB_ICONEXCLAMATION);
	}
	freopen_s(&fp, "CONOUT$", "w", stdout);
}
void Engine::activateWindows() {
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	ShowCursor(false);
}
void Engine::shutdownWindows() {
	ShowCursor(true);

	if (FULL_SCREEN) {
		ChangeDisplaySettings(NULL, 0);
	}
	DestroyWindow(hwnd);
	hwnd = NULL;

	UnregisterClass(app_name, hinstance);
	hinstance = NULL;

	ApplicationHandle = NULL;
}
void Engine::shutdownConsole() {
	if (fp) {
		fclose(fp);
		if (!FreeConsole()) {
			MessageBox(NULL, "Failed to free the console!", NULL, MB_ICONEXCLAMATION);
		}
	}
}
void Engine::shutdown() {
	gameEnd();

	if (renderer) {
		renderer->shutdown();
		delete renderer;
		renderer = 0;
	}
	if (im) {
		im->shutdown();
		delete im;
		im = 0;
	}
	/*if (em) {
		delete em;
		em = 0;
	}*/
	shutdownWindows();
	shutdownConsole();
}
LRESULT CALLBACK Engine::MessageHandler(HWND hwnd, UINT msg_code, WPARAM wparam, LPARAM lparam) {
	switch (msg_code) {
		/*case WM_KEYDOWN:	im->setKeyState((unsigned int)wparam, true);
							return 0;
		case WM_KEYUP:		im->setKeyState((unsigned int)wparam, false);
							return 0;
		case WM_ACTIVATE:	if (im) { im->setIsFocused((unsigned int)wparam); }
							return 0;*/
		default:			return DefWindowProc(hwnd, msg_code, wparam, lparam);
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}