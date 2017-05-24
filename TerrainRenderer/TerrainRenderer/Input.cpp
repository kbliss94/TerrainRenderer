#include "Input.h"

namespace TerrainRenderer
{
	Input::Input()
	{
		//call initialize?
	}

	Input::Input(const Input& rhs)
	{

	}

	Input& Input::operator=(const Input& rhs)
	{
		return *this;
	}

	Input::~Input()
	{

	}

	void Input::Initialize()
	{
		for (int i = 0; i < mKeyAmount; ++i)
		{
			mKeys[i] = false;
		}
	}

	void Input::KeyDown(unsigned int input)
	{
		if (0 <= input && input < mKeyAmount)
		{
			mKeys[input] = true;
		}
	}

	void Input::KeyUp(unsigned int input)
	{
		if (0 <= input && input < mKeyAmount)
		{
			mKeys[input] = false;
		}
	}

	bool Input::IsKeyDown(unsigned int key)
	{
		if (0 <= key && key < mKeyAmount)
		{
			return mKeys[key];
		}
		
		return false;
	}
}