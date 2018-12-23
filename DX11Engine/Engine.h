#ifndef ENGINE_H
#define ENGINE_H

#include <windows.h>

#include <string>

#include "InputManager.h"
#include "Renderer.h"

class Engine {
	public:	
		Engine();
		Engine(const Engine&);
		~Engine();

		virtual bool gameStart() = 0;
		virtual bool gameUpdate(float) = 0;
		virtual bool gameDraw() = 0;
		virtual void gameEnd() = 0;

		bool init();
		void shutdown();
		void run();

		LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	private:
		bool update();
		void initWindows(int&, int&);
		void initConsole();
		void activateWindows();
		void shutdownWindows();
		void shutdownConsole();

		float delta_time;

		LPCSTR app_name;
		HINSTANCE hinstance;
		HWND hwnd;
		FILE* fp;

	protected:
		InputManager* im;
		//EntityManager* em;
		//AssetManager* am;

		Renderer* renderer;

		int m_screen_width;
		int m_screen_height;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static Engine* ApplicationHandle = 0;

#endif
