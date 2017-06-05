#include "Font.h"

namespace TerrainRenderer
{
	Font::Font():
		mFont(nullptr), mTexture(nullptr)
	{

	}

	Font::Font(const Font& rhs)
	{

	}

	Font& Font::operator=(const Font& rhs)
	{
		return *this;
	}

	Font::~Font()
	{

	}

	bool Font::Initialize(ID3D11Device* device, char* fontFilename, WCHAR* textureFilename)
	{
		bool result;

		//loading the text file containing the font data
		result = LoadFontData(fontFilename);
		if (!result)
		{
			return false;
		}

		//loading the texture with the font characters on it
		result = LoadTexture(device, textureFilename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Font::Shutdown()
	{
		ReleaseTexture();
		ReleaseFontData();
	}

	bool Font::LoadFontData(char* filename)
	{
		ifstream fin;
		char temp;

		//setting up the font spacing buffer
		mFont = new FontType[mNumCharsInTexture];
		if (!mFont)
		{
			return false;
		}

		//reading in the font size & spacing b/w chars
		fin.open(filename);
		if (fin.fail())
		{
			return false;
		}

		for (int i = 0; i < mNumCharsInTexture; ++i)
		{
			fin.get(temp);
			while (temp != ' ')
			{
				fin.get(temp);
			}

			fin.get(temp);
			while (temp != ' ')
			{
				fin.get(temp);
			}

			fin >> mFont[i].left;
			fin >> mFont[i].right;
			fin >> mFont[i].size;
		}

		fin.close();

		return true;
	}

	void Font::ReleaseFontData()
	{
		if (mFont)
		{
			delete[] mFont;
			mFont = nullptr;
		}
	}

	bool Font::LoadTexture(ID3D11Device* device, WCHAR* filename)
	{
		bool result;

		//setting up the Texture object
		mTexture = new Texture;
		if (!mTexture)
		{
			return false;
		}

		result = mTexture->Initialize(device, filename);
		if (!result)
		{
			return false;
		}

		return true;
	}

	void Font::ReleaseTexture()
	{
		if (mTexture)
		{
			mTexture->Shutdown();
			delete mTexture;
			mTexture = nullptr;
		}
	}

	ID3D11ShaderResourceView* Font::GetTexture()
	{
		return mTexture->GetTexture();
	}

	void Font::BuildVertexArray(void* vertices, char* sentence, float drawX, float drawY)
	{
		VertexType* vertexPtr;
		int numLetters, index, letter;

		vertexPtr = static_cast<VertexType*>(vertices);
		numLetters = static_cast<int>(strlen(sentence));
		index = 0;

		//drawing each letter onto a quad
		for (int i = 0; i < numLetters; ++i)
		{
			letter = static_cast<int>(sentence[i]) - 32;

			if (letter == 0)
			{
				drawX = drawX + 3.0f;
			}
			else
			{
				//first triangle in quad
				vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);	//top left
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].left, 0.0f);
				index++;

				vertexPtr[index].position = D3DXVECTOR3((drawX + mFont[letter].size), (drawY - 16), 0.0f);	//bottom right
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].right, 1.0f);
				index++;

				vertexPtr[index].position = D3DXVECTOR3(drawX, (drawY - 16), 0.0f);	//bottom left
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].left, 1.0f);
				index++;

				//second triangle in quad
				vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0.0f);	//top left
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].left, 0.0f);
				index++;

				vertexPtr[index].position = D3DXVECTOR3((drawX + mFont[letter].size), drawY, 0.0f);	//top right
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].right, 0.0f);
				index++;

				vertexPtr[index].position = D3DXVECTOR3((drawX + mFont[letter].size), (drawY - 16), 0.0f);	//bottom right
				vertexPtr[index].texture = D3DXVECTOR2(mFont[letter].right, 1.0f);
				index++;

				drawX = drawX + mFont[letter].size + 1.0f;
			}
		}
	}
}