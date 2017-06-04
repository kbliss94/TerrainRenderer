#include "Camera.h"

namespace TerrainRenderer
{
	Camera::Camera():
		mPositionX(0), mPositionY(0), mPositionZ(0), mRotationX(0), mRotationY(0), mRotationZ(0)
	{

	}

	Camera::Camera(const Camera& rhs)
	{

	}

	Camera& Camera::operator=(const Camera& rhs)
	{
		return *this;
	}

	Camera::~Camera()
	{

	}

	void Camera::SetPosition(float x, float y, float z)
	{
		mPositionX = x;
		mPositionY = y;
		mPositionZ = z;
	}

	void Camera::SetRotation(float x, float y, float z)
	{
		mRotationX = x;
		mRotationY = y;
		mRotationZ = z;
	}

	D3DXVECTOR3 Camera::GetPosition()
	{
		return D3DXVECTOR3(mPositionX, mPositionY, mPositionZ);
	}

	D3DXVECTOR3 Camera::GetRotation()
	{
		return D3DXVECTOR3(mRotationX, mRotationY, mRotationZ);
	}

	void Camera::Render()
	{
		D3DXVECTOR3 up, position, lookAt;
		D3DXMATRIX rotationMatrix;
		float yaw, pitch, roll;

		up.x = 0.0f;
		up.y = 1.0f;
		up.z = 0.0f;

		position.x = mPositionX;
		position.y = mPositionY;
		position.z = mPositionZ;

		lookAt.x = 0.0f;
		lookAt.y = 0.0f;
		lookAt.z = 1.0f;

		pitch = mRotationX * mRadiansMultiplier;
		yaw = mRotationY * mRadiansMultiplier;
		roll = mRotationZ * mRadiansMultiplier;

		D3DXMatrixRotationYawPitchRoll(&rotationMatrix, yaw, pitch, roll);

		//transforming lookAt & up so the view is correctly rotated at the origin
		D3DXVec3TransformCoord(&lookAt, &lookAt, &rotationMatrix);
		D3DXVec3TransformCoord(&up, &up, &rotationMatrix);

		//translating the rotated camera position to the location of the viewer
		lookAt = position + lookAt;

		D3DXMatrixLookAtLH(&mViewMatrix, &position, &lookAt, &up);
	}

	void Camera::GetViewMatrix(D3DXMATRIX& viewMatrix)
	{
		viewMatrix = mViewMatrix;
	}
}