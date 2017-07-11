#include "Light.h"

namespace TerrainRenderer
{
	Light::Light()
	{

	}

	Light::Light(const Light& rhs)
	{

	}

	Light& Light::operator=(const Light& rhs)
	{
		return *this;
	}

	Light::~Light()
	{

	}

	void Light::SetAmbientColor(float red, float green, float blue, float alpha)
	{
		mAmbientColor = D3DXVECTOR4(red, green, blue, alpha);
	}

	void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
	{
		mDiffuseColor = D3DXVECTOR4(red, green, blue, alpha);
	}

	void Light::SetDirection(float x, float y, float z)
	{
		mDirection = D3DXVECTOR3(x, y, z);
	}

	D3DXVECTOR4 Light::GetAmbientColor()
	{
		return mAmbientColor;
	}

	D3DXVECTOR4 Light::GetDiffuseColor()
	{
		return mDiffuseColor;
	}

	D3DXVECTOR3 Light::GetDirection()
	{
		return mDirection;
	}
}