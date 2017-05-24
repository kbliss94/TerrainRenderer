#pragma once
#include <windows.h>
#include "Graphics.h"
#include "Input.h"

#ifndef _SYSTEMCLASS_
#define _SYSTEMCLASS_

#define WIN32_LEAN_AND_MEAN

using namespace std;

namespace TerrainRenderer
{
	//!Global
	static System* ApplicationHandle = 0;

	class System
	{
	public:
		System();
		System(const System& rhs);
		System& operator=(const System& rhs);
		~System();

		bool Initialize();
		void Run();
		void Shutdown();

		LRESULT CALLBACK MessageHandler(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam);

	private:
		bool Frame();
		/////figure out what to name these vars
		void InitializeWindows(int& screenWidth, int& screenHeight);
		void ShutdownWindows();

		LPCWSTR mApplicationName;
		HINSTANCE mHInstance;
		HWND mHWND;

		Input* mInput;
		Graphics* mGraphics;
	};

	//!Function prototypes
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam);
}

#endif
