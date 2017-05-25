#ifndef _INPUTCLASS_
#define _INPUTCLASS_

#pragma once

using namespace std;

namespace TerrainRenderer
{
	class Input final
	{
	public:
		//!Constructor
		Input();

		//!Copy constructor
		Input(const Input& rhs);

		//!Assignment operator
		Input& operator=(const Input& rhs);

		//!Destructor
		~Input();

		void Initialize();

		//!Records true in the key array if the corresponding key was pressed
		void KeyDown(unsigned int input);

		//!Records false in the key array if the corresponding key was released
		void KeyUp(unsigned int input);

		//!Returns state of the key specified
		/*!
		\return true if the key was pressed & false if the key was not pressed
		*/
		bool IsKeyDown(unsigned int key);

	private:
		const static int mKeyAmount = 256;
		bool mKeys[mKeyAmount];
	};
}

#endif
