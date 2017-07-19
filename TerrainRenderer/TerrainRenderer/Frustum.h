#pragma once

#include <d3dx10math.h>

namespace TerrainRenderer
{
	class Frustum
	{
	public:
		Frustum();
		Frustum(const Frustum& rhs);
		~Frustum();

		//!Called every frame by Graphics; calculates the matrix of the view frustum at each frame
		void ConstructFrustum(float screenDepth, D3DXMATRIX projectionMatrix, D3DXMATRIX viewMatrix);

		//!Checks whether or not a single point is within the viewing frustum
		bool CheckPoint(float x, float y, float z);

		//!Checks whether or not any of the eight corner points of the cube are within the viewing frustum
		bool CheckCube(float xCenter, float yCenter, float zCenter, float radius);

		//!Checks whether or not the radius of the sphere from the center point is within the viewing frustum
		bool CheckSphere(float xCenter, float yCenter, float zCenter, float radius);

		//!Checks whether or not the 8 corner points of the rectangle are within the viewing frustum
		bool CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);

	private:
		D3DXPLANE mPlanes[6];
	};
}
