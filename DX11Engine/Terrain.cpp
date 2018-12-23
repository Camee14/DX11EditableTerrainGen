#include "Terrain.h"

using namespace DirectX;

struct VBO {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;

	float padding;
};

bool loadTerrain(HeightMap h, ID3D11Device* device, Model* m) {
	bool res = false;

	std::vector<VBO> vertices;
	std::vector<unsigned long> indices;

	for (int x = 0; x < h.getWidth(); x ++) {
		for (int z = 0; z < h.getHeight(); z ++) {
			VBO vbo;
			vbo.position.x = x;
			vbo.position.y = h.getValue(x, z);
			vbo.position.z = z;

			vbo.uv.x = x;
			vbo.uv.y = z;

			vertices.push_back(vbo);
		}
	}
	unsigned long index = 0;
	for (int i = 0; i < (vertices.size() - h.getWidth()); i++) {
		if (i % h.getWidth() == h.getWidth() - 1) {
			index++;
			continue;
		}
		indices.push_back(index + h.getWidth() + 1);
		indices.push_back(index + h.getWidth());
		indices.push_back(index);
		
		indices.push_back(index + 1);
		indices.push_back(index + h.getWidth() + 1);
		indices.push_back(index);

		index++;
	}
	for (int i = 0; i < indices.size(); i += 3) {
		XMFLOAT3 vec1;
		vec1.x = vertices[indices[i + 1]].position.x - vertices[indices[i]].position.x;
		vec1.y = vertices[indices[i + 1]].position.y - vertices[indices[i]].position.y;
		vec1.z = vertices[indices[i + 1]].position.z - vertices[indices[i]].position.z;

		XMFLOAT3 vec2;
		vec2.x = vertices[indices[i + 2]].position.x - vertices[indices[i]].position.x;
		vec2.y = vertices[indices[i + 2]].position.y - vertices[indices[i]].position.y;
		vec2.z = vertices[indices[i + 2]].position.z - vertices[indices[i]].position.z;

		XMVECTOR normal = XMVector3Cross(XMLoadFloat3(&vec1), XMLoadFloat3(&vec2));
		
		XMStoreFloat3(&vertices[indices[i]].normal, normal);
		XMStoreFloat3(&vertices[indices[i + 1]].normal, normal);
		XMStoreFloat3(&vertices[indices[i + 1]].normal, normal);
	}
	/*for (int i = 0; i < vertices.size(); i++) {
		short num_connections;
		unsigned long connected_indices[6];

		int pos_in_row = i % h.getWidth();
		if (pos_in_row == 0) {
		
		}
		else if (pos_in_row == h.getWidth() - 1) {
		
		}
		else {
		
		}
	}*/

	printf("num vertices: %d, num indices: %d \n", vertices.size(), indices.size());
	res = m->init(device, vertices, indices);
	if (!res) {
		return false;
	}
	return true;
}
XMFLOAT3 getSurfaceNormal(XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c) {
	XMFLOAT3 vec1;
	vec1.x = b.x - a.x;
	vec1.y = b.y - a.y;
	vec1.z = b.z - a.z;

	XMFLOAT3 vec2;
	vec2.x = c.x - a.x;
	vec2.y = c.y - a.y;
	vec2.z = c.z - a.z;

	XMVECTOR normal = XMVector3Cross(XMLoadFloat3(&vec1), XMLoadFloat3(&vec2));
	XMFLOAT3 res;
	XMStoreFloat3(&res, normal);

	return res;
}