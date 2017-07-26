#include "QuadTree.h"

namespace TerrainRenderer
{
	QuadTree::QuadTree():
		mTerrain(nullptr), mParentNode(nullptr), mVertexList(nullptr)
	{

	}


	QuadTree::QuadTree(const QuadTree& rhs)
	{
	}


	QuadTree::~QuadTree()
	{
	}


	bool QuadTree::Initialize(std::shared_ptr<Terrain> terrain, ID3D11Device* device)
	{
		int vertexCount;
		float centerX, centerZ, width;

		mDevice = device;

		//getting necessary info from terrain (# of vertices in the terrain)
			//uses that # to create a vertex list that's sent to Terrain & filled w/the terrain vertex info

		// Get the number of vertices in the terrain vertex array.
		vertexCount = terrain->GetVertexCount();

		// Store the total triangle count for the vertex list.
		mTriangleCount = vertexCount / 3;

		// Create a vertex array to hold all of the terrain vertices.
		mVertexList = new VertexType[vertexCount];
		if (!mVertexList)
		{
			return false;
		}

		mTerrain = terrain;
		if (!mTerrain)
		{
			return false;
		}

		// Copy the terrain vertices into the vertex list.
		mTerrain->CopyVertexArray((void*)mVertexList);

		//after vertex list is filled w/the terrain info it can calculate the dimensions of the parent node & start
			//the recursive method of building the quad tree

		// Calculate the center x,z and the width of the mesh.
		CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

		// Create the parent node for the quad tree.
		mParentNode = new NodeType;
		if (!mParentNode)
		{
			return false;
		}

		// Recursively build the quad tree based on the vertex list data and mesh dimensions.
		CreateTreeNode(mParentNode, centerX, centerZ, width, device);

		// Release the vertex list since the quad tree now has the vertices in each node.
		if (mVertexList)
		{
			delete[] mVertexList;
			mVertexList = 0;
		}

		return true;
	}

	void QuadTree::Shutdown()
	{
		// Recursively release the quad tree data.
		if (mParentNode)
		{
			ReleaseNode(mParentNode);
			delete mParentNode;
			mParentNode = nullptr;
		}
	}

	//!Calls RenderNode which goes through the tree & renders nodes using the frustum object which determines node visibility
	/*!
	mDrawCount must be initialized to zero before the rendering is done, bc it will be incremented for each node drawn for all the triangles in each node
	*/
	void QuadTree::Render(Frustum* frustum, ID3D11DeviceContext* deviceContext, TerrainShader* shader)
	{
		// Reset the number of triangles that are drawn for this frame.
		mTriDrawCount = 0;

		// Render each node that is visible starting at the parent node and moving down the tree.
		RenderNode(mParentNode, frustum, deviceContext, shader);
	}

	//!Returns the total number of triangles that were drawn in the Render function call
	int QuadTree::GetTriDrawCount()
	{
		return mTriDrawCount;
	}

	int QuadTree::GetTriTotalCount()
	{
		return mTriangleCount;
	}

	void QuadTree::UpdateTerrainData()
	{
		int vertexCount;
		float centerX, centerZ, width;

		// Get the number of vertices in the terrain vertex array.
		vertexCount = mTerrain->GetVertexCount();

		// Store the total triangle count for the vertex list.
		mTriangleCount = vertexCount / 3;

		// Create a vertex array to hold all of the terrain vertices.
		mVertexList = new VertexType[vertexCount];

		// Copy the terrain vertices into the vertex list.
		mTerrain->CopyVertexArray((void*)mVertexList);

		//after vertex list is filled w/the terrain info it can calculate the dimensions of the parent node & start
		//the recursive method of building the quad tree

		// Calculate the center x,z and the width of the mesh.
		CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

		// Create the parent node for the quad tree.
		Shutdown();
		mParentNode = new NodeType;

		// Recursively build the quad tree based on the vertex list data and mesh dimensions.
		CreateTreeNode(mParentNode, centerX, centerZ, width, mDevice);

		// Release the vertex list since the quad tree now has the vertices in each node.
		if (mVertexList)
		{
			delete[] mVertexList;
			mVertexList = 0;
		}
	}

	//!Determines the physical quad size of the parent node
	/*!
	Goes through all the vertices in the terrain vertex list & uses them to calculate the center of the terrain as well as the absolute max width of the terrain.
	These values will be used later to create the size of the parent node
	*/
	void QuadTree::CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth)
	{
		float maxWidth, maxDepth, minWidth, minDepth, width, depth, maxX, maxZ;

		// Initialize the center position of the mesh to zero.
		centerX = 0.0f;
		centerZ = 0.0f;

		// Sum all the vertices in the mesh.
		for (int i = 0; i < vertexCount; i++)
		{
			centerX += mVertexList[i].position.x;
			centerZ += mVertexList[i].position.z;
		}

		// And then divide it by the number of vertices to find the mid-point of the mesh.
		centerX = centerX / (float)vertexCount;
		centerZ = centerZ / (float)vertexCount;

		// Initialize the maximum and minimum size of the mesh.
		maxWidth = 0.0f;
		maxDepth = 0.0f;

		minWidth = fabsf(mVertexList[0].position.x - centerX);
		minDepth = fabsf(mVertexList[0].position.z - centerZ);

		// Go through all the vertices and find the maximum and minimum width and depth of the mesh.
		for (int i = 0; i < vertexCount; i++)
		{
			width = fabsf(mVertexList[i].position.x - centerX);
			depth = fabsf(mVertexList[i].position.z - centerZ);

			if (width > maxWidth) { maxWidth = width; }
			if (depth > maxDepth) { maxDepth = depth; }
			if (width < minWidth) { minWidth = width; }
			if (depth < minDepth) { minDepth = depth; }
		}

		// Find the absolute maximum value between the min and max depth and width.
		maxX = (float)max(fabs(minWidth), fabs(maxWidth));
		maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

		// Calculate the maximum diameter of the mesh.
		meshWidth = max(maxX, maxZ) * 2.0f;
	}

	//!Recursively builds the quad tree
	void QuadTree::CreateTreeNode(NodeType* node, float positionX, float positionZ, float width, ID3D11Device* device)
	{
		int numTriangles, count, vertexCount, index, vertexIndex;
		float offsetX, offsetZ;
		VertexType* vertices;
		unsigned long* indices;
		bool result;
		D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData, indexData;

		// Store the node position and size.
		node->positionX = positionX;
		node->positionZ = positionZ;
		node->width = width;

		// Initialize the triangle count to zero for the node.
		node->triangleCount = 0;

		// Initialize the vertex and index buffer to null.
		node->vertexBuffer = 0;
		node->indexBuffer = 0;

		// Initialize the children nodes of this node to null.
		node->nodes[0] = 0;
		node->nodes[1] = 0;
		node->nodes[2] = 0;
		node->nodes[3] = 0;

		// Count the number of triangles that are inside this node.
		numTriangles = CountTriangles(positionX, positionZ, width);

		//CASE 1: if there are no triangles in this node, then this part of the tree is complete. Returns the node as empty & requires no processing
		if (numTriangles == 0)
		{
			return;
		}

		//CASE 2: if there are too many triangles inside this node, it gets split into 4 new quads/nodes
		if (numTriangles > MAX_TRIANGLES)
		{
			for (int i = 0; i < 4; i++)
			{
				// Calculate the position offsets for the new child node.
				offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
				offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

				// See if there are any triangles in the new node.
				count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
				if (count > 0)
				{
					// If there are triangles inside where this new node would be then create the child node.
					node->nodes[i] = new NodeType;

					// Extend the tree starting from this new child node now.
					CreateTreeNode(node->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
				}
			}

			return;
		}

		//CASE 3: if there is the right # of triangles, then create & load the vertex & index buffers from the terrain list into this node
			//Also means that this must be a bottom child node (which means we must create the list of triangles to store in it)
		node->triangleCount = numTriangles;

		// Calculate the number of vertices.
		vertexCount = numTriangles * 3;

		// Create the vertex array.
		vertices = new VertexType[vertexCount];

		// Create the index array.
		indices = new unsigned long[vertexCount];

		// Initialize the index for this new vertex and index array.
		index = 0;

		// Go through all the triangles in the vertex list.
		for (int i = 0; i < mTriangleCount; i++)
		{
			// If the triangle is inside this node then add it to the vertex array.
			result = IsTriangleContained(i, positionX, positionZ, width);
			if (result == true)
			{
				// Calculate the index into the terrain vertex list.
				vertexIndex = i * 3;

				// Get the three vertices of this triangle from the vertex list.
				vertices[index].position = mVertexList[vertexIndex].position;
				vertices[index].texture = mVertexList[vertexIndex].texture;
				vertices[index].normal = mVertexList[vertexIndex].normal;
				indices[index] = index;
				index++;

				vertexIndex++;
				vertices[index].position = mVertexList[vertexIndex].position;
				vertices[index].texture = mVertexList[vertexIndex].texture;
				vertices[index].normal = mVertexList[vertexIndex].normal;
				indices[index] = index;
				index++;

				vertexIndex++;
				vertices[index].position = mVertexList[vertexIndex].position;
				vertices[index].texture = mVertexList[vertexIndex].texture;
				vertices[index].normal = mVertexList[vertexIndex].normal;
				indices[index] = index;
				index++;
			}
		}

		// Set up the description of the vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now finally create the vertex buffer.
		device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->vertexBuffer);

		// Set up the description of the index buffer.
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		device->CreateBuffer(&indexBufferDesc, &indexData, &node->indexBuffer);

		// Release the vertex and index arrays now that the data is stored in the buffers in the node.
		delete[] vertices;
		vertices = 0;

		delete[] indices;
		indices = 0;
	}

	//!Goes through the list of triangles from the terrain data & determines which ones are inside the parameter dimensions 
	int QuadTree::CountTriangles(float positionX, float positionZ, float width)
	{
		int count;
		bool result;

		// Initialize the count to zero.
		count = 0;

		// Go through all the triangles in the entire mesh and check which ones should be inside this node.
		for (int i = 0; i < mTriangleCount; i++)
		{
			// If the triangle is inside the node then increment the count by one.
			result = IsTriangleContained(i, positionX, positionZ, width);
			if (result == true)
			{
				count++;
			}
		}

		return count;
	}

	//!Calculates whether or not the given triangle is completely inside the input cube dimensions
	bool QuadTree::IsTriangleContained(int index, float positionX, float positionZ, float width)
	{
		float radius;
		int vertexIndex;
		float x1, z1, x2, z2, x3, z3;
		float minimumX, maximumX, minimumZ, maximumZ;

		// Calculate the radius of this node.
		radius = width / 2.0f;

		// Get the index into the vertex list.
		vertexIndex = index * 3;

		// Get the three vertices of this triangle from the vertex list.
		x1 = mVertexList[vertexIndex].position.x;
		z1 = mVertexList[vertexIndex].position.z;
		vertexIndex++;

		x2 = mVertexList[vertexIndex].position.x;
		z2 = mVertexList[vertexIndex].position.z;
		vertexIndex++;

		x3 = mVertexList[vertexIndex].position.x;
		z3 = mVertexList[vertexIndex].position.z;

		// Check to see if the minimum of the x coordinates of the triangle is inside the node.
		minimumX = min(x1, min(x2, x3));
		if (minimumX > (positionX + radius))
		{
			return false;
		}

		// Check to see if the maximum of the x coordinates of the triangle is inside the node.
		maximumX = max(x1, max(x2, x3));
		if (maximumX < (positionX - radius))
		{
			return false;
		}

		// Check to see if the minimum of the z coordinates of the triangle is inside the node.
		minimumZ = min(z1, min(z2, z3));
		if (minimumZ > (positionZ + radius))
		{
			return false;
		}

		// Check to see if the maximum of the z coordinates of the triangle is inside the node.
		maximumZ = max(z1, max(z2, z3));
		if (maximumZ < (positionZ - radius))
		{
			return false;
		}

		return true;
	}

	//!Recursively releases all nodes & the data they contain
	void QuadTree::ReleaseNode(NodeType* node)
	{
		// Recursively go down the tree and release the bottom nodes first.
		for (int i = 0; i < 4; i++)
		{
			if (node->nodes[i] != 0)
			{
				ReleaseNode(node->nodes[i]);
			}
		}

		// Release the vertex buffer for this node.
		if (node->vertexBuffer)
		{
			node->vertexBuffer->Release();
			node->vertexBuffer = 0;
		}

		// Release the index buffer for this node.
		if (node->indexBuffer)
		{
			node->indexBuffer->Release();
			node->indexBuffer = 0;
		}

		// Release the four child nodes.
		for (int i = 0; i < 4; i++)
		{
			if (node->nodes[i])
			{
				delete node->nodes[i];
				node->nodes[i] = 0;
			}
		}
	}

	//!Draws all visible nodes in the quad tree
	void QuadTree::RenderNode(NodeType* node, Frustum* frustum, ID3D11DeviceContext* deviceContext, TerrainShader* shader)
	{
		bool result;
		int count, indexCount;
		unsigned int stride, offset;

		// Check to see if the node can be viewed, height doesn't matter in a quad tree.
		result = frustum->CheckCube(node->positionX, 0.0f, node->positionZ, (node->width / 2.0f));

		// If it can't be seen then none of its children can either so don't continue down the tree, this is where the speed is gained.
		if (!result)
		{
			return;
		}

		// If it can be seen then check all four child nodes to see if they can also be seen.
		count = 0;

		for (int i = 0; i < 4; i++)
		{
			if (node->nodes[i] != 0)
			{
				count++;
				RenderNode(node->nodes[i], frustum, deviceContext, shader);
			}
		}

		// If there were any children nodes then there is no need to continue as parent nodes won't contain any triangles to render.
		if (count != 0)
		{
			return;
		}

		// Otherwise, if this node can be seen and has triangles in it then render these triangles.

		// Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
		offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetVertexBuffers(0, 1, &node->vertexBuffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		deviceContext->IASetIndexBuffer(node->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Determine the number of indices in this node.
		indexCount = node->triangleCount * 3;

		// Call the terrain shader to render the polygons in this node.
		shader->RenderShader(deviceContext, indexCount);

		// Increase the count of the number of polygons that have been rendered during this frame.
		mTriDrawCount += node->triangleCount;
	}
}