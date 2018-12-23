#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class InputManager {
public:
	InputManager();
	InputManager(const InputManager&);
	~InputManager();

	bool init(int, int, HWND, HINSTANCE);
	void shutdown();
	bool update();

	bool onKeyDown(unsigned int);
	bool onKeyUp(unsigned int);
	bool onKeyPressed(unsigned int);

	void onMouseMove(int&, int&);
	void getMousePos(int&, int&);


private:
	unsigned char m_keys[256];
	unsigned char m_prev_keys[256];

	DIMOUSESTATE m_mouse_state;

	int m_mouse_x;
	int m_mouse_y;

	int m_prev_mouse_x;
	int m_prev_mouse_y;

	int m_screen_width;
	int m_screen_height;

	IDirectInput8* m_dinput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;
};


#endif