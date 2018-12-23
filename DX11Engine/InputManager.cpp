#include <windows.h>

#include "InputManager.h"

InputManager::InputManager() {
	m_mouse_x = 0;
	m_mouse_y = 0;

	m_prev_mouse_x = 0;
	m_prev_mouse_y = 0;

	m_dinput = 0;
	m_keyboard = 0;
	m_mouse = 0;
}
InputManager::InputManager(const InputManager& other) {

}
InputManager::~InputManager() {

}
bool InputManager::init(int screen_width, int screen_height, HWND window, HINSTANCE instance) {
	HRESULT res;
	
	m_screen_width = screen_width;
	m_screen_height = screen_height;

	res = DirectInput8Create(instance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_dinput, NULL);
	if (FAILED(res)) {
		return false;
	}

	res = m_dinput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if (FAILED(res)) {
		return false;
	}

	res = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(res)) {
		return false;
	}

	res = m_keyboard->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(res)) {
		return false;
	}

	res = m_keyboard->Acquire();
	if (FAILED(res)) {
		return false;
	}

	res = m_dinput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(res)) {
		return false;
	}

	res = m_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(res)) {
		return false;
	}

	res = m_mouse->SetCooperativeLevel(window, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(res)) {
		return false;
	}

	res = m_mouse->Acquire();
	if (FAILED(res)) {
		return false;
	}

	return true;
}
bool InputManager::update() {
	HRESULT res;

	memcpy(m_prev_keys, m_keys, 256);

	res = m_keyboard->GetDeviceState(sizeof(m_keys), (LPVOID)&m_keys);
	if (FAILED(res)) {
		if (res == DIERR_INPUTLOST || res == DIERR_NOTACQUIRED) {
			m_keyboard->Acquire();
		}
		else {
			return false;
		}
	}

	res = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouse_state);
	if (FAILED(res)) {
		if (res == DIERR_INPUTLOST || res == DIERR_NOTACQUIRED) {
			m_mouse->Acquire();
		}
		else {
			return false;
		}
	}

	m_prev_mouse_x = m_mouse_x;
	m_prev_mouse_y = m_mouse_y;

	m_mouse_x += m_mouse_state.lX;
	m_mouse_y += m_mouse_state.lY;

	/*if (m_mouse_x < 0) {
		m_mouse_x = 0;
	}
	if (m_mouse_y < 0) {
		m_mouse_y = 0;
	}

	if (m_mouse_x > m_screen_width) {
		m_mouse_x = m_screen_width;
	}
	if (m_mouse_y > m_screen_height) {
		m_mouse_y = m_screen_height;
	}*/

	return true;
}
void InputManager::shutdown() {
	if (m_mouse) {
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}
	if (m_keyboard) {
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}
	if (m_dinput) {
		m_dinput->Release();
		m_dinput = 0;
	}
}
//NOTE: key is down if high bit in byte is 1;
bool InputManager::onKeyDown(unsigned int key_code) {
	return (m_keys[key_code] & 0x80) && !(m_prev_keys[key_code] & 0x80);
}
bool InputManager::onKeyPressed(unsigned int key_code) {
	return (m_keys[key_code] & 0x80) && (m_prev_keys[key_code] & 0x80);
}
bool InputManager::onKeyUp(unsigned int key_code) {
	return !(m_keys[key_code] & 0x80) && (m_prev_keys[key_code] & 0x80);
}
void InputManager::onMouseMove(int& x, int& y) {
	x = m_mouse_x - m_prev_mouse_x;
	y = m_mouse_y - m_prev_mouse_y;
}
void InputManager::getMousePos(int& x, int& y) {
	x = m_mouse_x;
	y = m_mouse_y;
}