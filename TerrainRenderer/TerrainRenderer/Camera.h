#ifndef _CAMERA_
#define _CAMERA_

#pragma once

#include <D3DX10math.h>

using namespace std;

namespace TerrainRenderer
{
	class Camera
	{
	public:
		//!Constructor
		Camera();

		//!Copy constructor
		Camera(const Camera& rhs);

		//!Assignment operator
		Camera& operator=(const Camera& rhs);

		//!Destructor
		~Camera();

		void SetPosition(float x, float y, float z);
		void SetRotation(float x, float y, float z);

		D3DXVECTOR3 GetPosition();
		D3DXVECTOR3 GetRotation();

		void Render();
		void GetViewMatrix(D3DXMATRIX& viewMatrix);

	private:
		float mPositionX, mPositionY, mPositionZ;
		float mRotationX, mRotationY, mRotationZ;
		D3DXMATRIX mViewMatrix;
		const float mRadiansMultiplier = 0.0174532925f;
	};
}

#endif
