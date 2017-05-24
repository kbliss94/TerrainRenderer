#ifndef _SYSTEMCLASS_
#define _SYSTEMCLASS_

#pragma once
#include <windows.h>
#include "Graphics.h"
#include "Input.h"

#define WIN32_LEAN_AND_MEAN

using namespace std;

namespace TerrainRenderer
{
	//!Global
	static System* ApplicationHandle = 0;

	class System
	{
	public:
		//!Constructor
		System();

		//!Copy constructor
		System(const System& rhs);

		//!Assignment operator
		System& operator=(const System& rhs);

		//!Destructor
		~System();

		//!Creates the window that will be used
		/*!
		Creates & initializes mInput & mGraphics
		\return true if able to initialize & false otherwise
		*/
		bool Initialize();

		//!Does all application processing, which is done in Frame() which is called each loop
		void Run();

		void Shutdown();

		//!Listens for certain information
		/*!
		Reads if a key is pressed/released & passes that information to mInput
		*/
		LRESULT CALLBACK MessageHandler(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam);

	private:
		//!Handles application processing
		/*!
		\return true if mGraphics was able to process & false if the user pressed the Escape key
		*/
		bool Frame();

		//!Building the window we want to render to
		/*!
		Currently displays a plain black window with no borders
		*/
		void InitializeWindows(int& screenWidth, int& screenHeight);

		void ShutdownWindows();

		LPCWSTR mApplicationName;
		HINSTANCE mHInstance;
		HWND mHWND;

		Input* mInput;
		Graphics* mGraphics;
	};

	//!Function prototypes
	//!Sends all messages to MessageHandler()
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uint, WPARAM wparam, LPARAM lparam);
}

#endif
