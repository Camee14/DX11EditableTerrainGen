#include <stdio.h>
#include <vector>
#include <map>
//#include <random>
#include <time.h> 
#include "Chunk.h"
#include "Noise.h"

using namespace DirectX;

Chunk::Chunk() {
	m_can_render = false;
	m_is_init = false;
	m_is_air = false;
	m_destroy = false;
	m_vertex_buffer = 0;
	m_data = 0;
}
void Chunk::init(int x, int y, int z, int h, int offsetY) {
	m_int_pos.x = x * CHUNK_SIZE;
	m_int_pos.y = y * CHUNK_SIZE;
	m_int_pos.z = z * CHUNK_SIZE;
	transform.setPosition(m_int_pos.x * CHUNK_SCALE, (m_int_pos.y + offsetY) * CHUNK_SCALE, m_int_pos.z * CHUNK_SCALE);
	transform.setScale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);

	ImprovedPerlinNoise noise;
	
	int size = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
	double Hscale = 10;
	double Vscale = 4;
	m_data = new VOXEL[size];
	/*for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			for (int k = 0; k < CHUNK_SIZE; k++) {
				int index = getIndex(i, j, k);
				double x_coord = ((double)(x * CHUNK_SIZE + i) / CHUNK_SIZE) / Hscale;
				double y_coord = ((double)(y * CHUNK_SIZE + j) / CHUNK_SIZE) / Vscale;
				double z_coord = ((double)(z * CHUNK_SIZE + k) / CHUNK_SIZE) / Hscale;
				m_data[index].density = noise.getValue(x_coord, y_coord, z_coord, 8, 0.42) + 1.0 - ((double)(y * CHUNK_SIZE + j)) / ((double)CHUNK_SIZE * h);
				//m_data[index].density = 1;
			}
		}
	}*/
	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int k = 0; k < CHUNK_SIZE; k++) {
			double x_coord = ((double)(x * CHUNK_SIZE + i) / CHUNK_SIZE);
			double z_coord = ((double)(z * CHUNK_SIZE + k) / CHUNK_SIZE);
			double area = noise.getValue(x_coord / 35, z_coord / 35, 0.103);
			if (area < 0.1) {
				area = 0.1;
			}
			double height = noise.getValue(x_coord / 10, z_coord / 10, 0.1f, 4, 0.22);
			if (height < 0) {
				height *= -1;
			}
			for (int j = 0; j < CHUNK_SIZE; j++) {
				int index = getIndex(i, j, k);
					
				m_data[index].density = 1 - ((double)(m_int_pos.y + j) * 3.5) / ((double)(h) * CHUNK_SIZE);
				m_data[index].density += (height * 3.5) * area;
			}
		}
	}
	m_is_init = true;
}
void Chunk::setActive(ID3D11DeviceContext* context) {
	unsigned int stride;
	unsigned int offset;

	stride = m_stride;
	offset = 0;

	context->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
void Chunk::shutdown() {
	if (m_vertex_buffer) {
		m_vertex_buffer->Release();
		m_vertex_buffer = 0;
	}
	if (m_data) {
		delete[] m_data;
		m_data = 0;
	}
}
VOXEL Chunk::getVoxel(int x, int y, int z) {
	//printf("is init: %d", m_is_init);
	if (((x < 0 || x >= CHUNK_SIZE) || (y < 0 || y >= CHUNK_SIZE) || (z < 0 || z >= CHUNK_SIZE)) || !m_is_init) {
		VOXEL v;
		v.density = 0;
		return v;
	}
	return m_data[getIndex(x, y, z)];
}
void Chunk::setVoxel(int x, int y, int z, double d) {
	if (((x < 0 || x >= CHUNK_SIZE) || (y < 0 || y >= CHUNK_SIZE) || (z < 0 || z >= CHUNK_SIZE)) || !m_is_init) {
		return;
	}
	m_data[getIndex(x, y, z)].density = d;
}
void Chunk::refresh() {
	m_destroy = true;
}
bool Chunk::upload(ID3D11Device* device, unsigned int num, TRIANGLE* ts, ID3D11Buffer* p_buffer) {

	D3D11_BUFFER_DESC vertex_buffer_desc;
	D3D11_SUBRESOURCE_DATA vertex_data;
	HRESULT res;

	//std::map<int, std::vector<int> > point_map;

	std::vector<VBO> vbo;
	for (int i = 0; i < num; i++) {
		XMFLOAT3 vec1;
		vec1.x = ts[i].points[1].x - ts[i].points[0].x;
		vec1.y = ts[i].points[1].y - ts[i].points[0].y;
		vec1.z = ts[i].points[1].z - ts[i].points[0].z;

		XMFLOAT3 vec2;
		vec2.x = ts[i].points[2].x - ts[i].points[0].x;
		vec2.y = ts[i].points[2].y - ts[i].points[0].y;
		vec2.z = ts[i].points[2].z - ts[i].points[0].z;

		
		XMVECTOR normal = XMVector3Cross(XMLoadFloat3(&vec1), XMLoadFloat3(&vec2));
		normal = XMVector3Normalize(normal);

		for (int j = 0; j < 3; j++) {
			VBO v;
			v.position = ts[i].points[j];
			XMStoreFloat3(&v.normal, normal);

			vbo.push_back(v);

			/*(int index = getIndex(v.position.x, v.position.y, v.position.z);
			std::map<int, std::vector<int> >::iterator iter = point_map.find(index);
			if (iter != point_map.end()) 
			{

				iter->second.push_back(vbo.size() - 1);
			}
			else 
			{
				std::vector<int> vbos;
				vbos.push_back(vbo.size() - 1);
				point_map.emplace(std::pair<int, std::vector<int> >(index, vbos));
			}*/
		}
	}
	/*for (auto x : point_map)
	{
		XMFLOAT3 normal;
		normal.x = 0;
		normal.y = 0;
		normal.z = 0;
		for (int i : x.second) {
			//XMVector avec = XMVector2AngleBetweenNormals(XMLoadFloat3(&vec1), XMLoadFloat3(&vec2))
			normal.x += vbo[i].normal.x;
			normal.y += vbo[i].normal.y;
			normal.z += vbo[i].normal.z;
		}

		XMVECTOR nvec =  XMVector3Normalize(XMLoadFloat3(&normal));
		XMStoreFloat3(&normal, nvec);

		for (int i : x.second) {
			//vbo[i].normal = normal;
		}
	}*/

	m_vertex_count = vbo.size();

	if (m_vertex_count == 0) {
		return false;
	}

	m_stride = sizeof(VBO);

	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(VBO) * m_vertex_count;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	vertex_data.pSysMem = &vbo[0];
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	res = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &p_buffer);
	if (FAILED(res)) {
		return false;
	}
	std::swap(m_vertex_buffer, p_buffer);

	m_is_air = false;
	m_can_render = true;

	return true;
}
Chunk::~Chunk() {

}