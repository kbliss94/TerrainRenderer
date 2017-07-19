#include "Frustum.h"

namespace TerrainRenderer
{
	Frustum::Frustum()
	{
	}


	Frustum::Frustum(const Frustum& rhs)
	{
	}


	Frustum::~Frustum()
	{
	}

	void Frustum::ConstructFrustum(float screenDepth, D3DXMATRIX projectionMatrix, D3DXMATRIX viewMatrix)
	{
		float zMinimum, r;
		D3DXMATRIX matrix;

		// Calculate the minimum Z distance in the frustum.
		zMinimum = -projectionMatrix._43 / projectionMatrix._33;
		r = screenDepth / (screenDepth - zMinimum);
		projectionMatrix._33 = r;
		projectionMatrix._43 = -r * zMinimum;

		// Create the frustum matrix from the view matrix and updated projection matrix.
		D3DXMatrixMultiply(&matrix, &viewMatrix, &projectionMatrix);

		// Calculate near plane of frustum.
		mPlanes[0].a = matrix._14 + matrix._13;
		mPlanes[0].b = matrix._24 + matrix._23;
		mPlanes[0].c = matrix._34 + matrix._33;
		mPlanes[0].d = matrix._44 + matrix._43;
		D3DXPlaneNormalize(&mPlanes[0], &mPlanes[0]);

		// Calculate far plane of frustum.
		mPlanes[1].a = matrix._14 - matrix._13;
		mPlanes[1].b = matrix._24 - matrix._23;
		mPlanes[1].c = matrix._34 - matrix._33;
		mPlanes[1].d = matrix._44 - matrix._43;
		D3DXPlaneNormalize(&mPlanes[1], &mPlanes[1]);

		// Calculate left plane of frustum.
		mPlanes[2].a = matrix._14 + matrix._11;
		mPlanes[2].b = matrix._24 + matrix._21;
		mPlanes[2].c = matrix._34 + matrix._31;
		mPlanes[2].d = matrix._44 + matrix._41;
		D3DXPlaneNormalize(&mPlanes[2], &mPlanes[2]);

		// Calculate right plane of frustum.
		mPlanes[3].a = matrix._14 - matrix._11;
		mPlanes[3].b = matrix._24 - matrix._21;
		mPlanes[3].c = matrix._34 - matrix._31;
		mPlanes[3].d = matrix._44 - matrix._41;
		D3DXPlaneNormalize(&mPlanes[3], &mPlanes[3]);

		// Calculate top plane of frustum.
		mPlanes[4].a = matrix._14 - matrix._12;
		mPlanes[4].b = matrix._24 - matrix._22;
		mPlanes[4].c = matrix._34 - matrix._32;
		mPlanes[4].d = matrix._44 - matrix._42;
		D3DXPlaneNormalize(&mPlanes[4], &mPlanes[4]);

		// Calculate bottom plane of frustum.
		mPlanes[5].a = matrix._14 + matrix._12;
		mPlanes[5].b = matrix._24 + matrix._22;
		mPlanes[5].c = matrix._34 + matrix._32;
		mPlanes[5].d = matrix._44 + matrix._42;
		D3DXPlaneNormalize(&mPlanes[5], &mPlanes[5]);

		return;
	}

	bool Frustum::CheckPoint(float x, float y, float z)
	{
		// Check if the point is inside all six planes of the view frustum.
		for (int i = 0; i < 6; i++)
		{
			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3(x, y, z)) < 0.0f)
			{
				return false;
			}
		}

		return true;
	}

	bool Frustum::CheckCube(float xCenter, float yCenter, float zCenter, float radius)
	{
		// Check if any one point of the cube is in the view frustum.
		for (int i = 0; i < 6; i++)
		{
			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + radius), (yCenter - radius), (zCenter - radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + radius), (yCenter + radius), (zCenter - radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + radius), (yCenter - radius), (zCenter + radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + radius), (yCenter + radius), (zCenter + radius))) >= 0.0f)
			{
				continue;
			}

			return false;
		}

		return true;
	}

	bool Frustum::CheckSphere(float xCenter, float yCenter, float zCenter, float radius)
	{
		// Check if the radius of the sphere is inside the view frustum.
		for (int i = 0; i < 6; i++)
		{
			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3(xCenter, yCenter, zCenter)) < -radius)
			{
				return false;
			}
		}

		return true;
	}

	bool Frustum::CheckRectangle(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
	{
		// Check if any of the 6 planes of the rectangle are inside the view frustum.
		for (int i = 0; i < 6; i++)
		{
			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			{
				continue;
			}

			if (D3DXPlaneDotCoord(&mPlanes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			{
				continue;
			}

			return false;
		}

		return true;
	}
}