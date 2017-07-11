#ifndef _LIGHT_
#define _LIGHT_

#pragma once

#include <D3DX10math.h>

namespace TerrainRenderer
{
	class Light
	{
	public:
		//!Constructor
		Light();

		//!Copy constructor
		Light(const Light& rhs);

		//!Assignment operator
		Light& operator=(const Light& rhs);

		//!Destructor
		~Light();

		void SetAmbientColor(float red, float green, float blue, float alpha);
		void SetDiffuseColor(float red, float green, float blue, float alpha);
		void SetDirection(float x, float y, float z);

		D3DXVECTOR4 GetAmbientColor();
		D3DXVECTOR4 GetDiffuseColor();
		D3DXVECTOR3 GetDirection();

	private:
		D3DXVECTOR4 mAmbientColor;
		D3DXVECTOR4 mDiffuseColor;
		D3DXVECTOR3 mDirection;
	};
}

#endif
