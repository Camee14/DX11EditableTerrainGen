#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "Transform.h"

enum primitives {
	PRIMITIVE_QUAD,
	PRIMITIVE_TRIANGLE,
};

struct Material {
	DirectX::XMFLOAT4 Ambient;		//16 bytes
	DirectX::XMFLOAT4 Diffuse;		//16 bytes
	DirectX::XMFLOAT4 Specular;		//16 bytes
	float SpecularPower;			//4 bytes
	//bool useTexture;				//1 byte
									//--11 bytes padding--
	DirectX::XMFLOAT3 padding_8;	//8 bytes
};

class Model {
public:
	Model();
	Model(const Model&);
	~Model();
	template<typename T>
	bool init(ID3D11Device* device, std::vector<T> vbo, std::vector<unsigned long> indices) {
		D3D11_BUFFER_DESC vertex_buffer_desc, index_buffer_desc;
		D3D11_SUBRESOURCE_DATA vertex_data, index_data;
		HRESULT res;

		m_vertex_count = vbo.size();
		m_index_count = indices.size();

		if (m_vertex_count == 0 || m_index_count == 0) {
			return false;
		}

		m_stride = sizeof(T);

		vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		vertex_buffer_desc.ByteWidth = sizeof(T) * m_vertex_count;
		vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertex_buffer_desc.CPUAccessFlags = 0;
		vertex_buffer_desc.MiscFlags = 0;
		vertex_buffer_desc.StructureByteStride = 0;

		vertex_data.pSysMem = &vbo[0];
		vertex_data.SysMemPitch = 0;
		vertex_data.SysMemSlicePitch = 0;

		res = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &m_vertex_buffer);
		if (FAILED(res)) {
			return false;
		}

		index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		index_buffer_desc.ByteWidth = sizeof(unsigned long) * m_index_count;
		index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		index_buffer_desc.CPUAccessFlags = 0;
		index_buffer_desc.MiscFlags = 0;
		index_buffer_desc.StructureByteStride = 0;

		index_data.pSysMem = &indices[0];
		index_data.SysMemPitch = 0;
		index_data.SysMemSlicePitch = 0;

		res = device->CreateBuffer(&index_buffer_desc, &index_data, &m_index_buffer);
		if (FAILED(res)) {
			return false;
		}

		return true;
	}
	void shutdown();

	Transform transform;
	Material material;

	int getIndexCount();
	void setActive(ID3D11DeviceContext*);
private:
	ID3D11Buffer *m_vertex_buffer, *m_index_buffer;
	int m_vertex_count, m_index_count, m_stride;
};

bool loadPrimitive(int type, ID3D11Device*, Model*);
bool loadModelFromFile(std::string, ID3D11Device*, Model*);
bool loadRect(ID3D11Device*, float, float, bool, Model*);

#endif
