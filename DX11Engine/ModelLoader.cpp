#include "ModelLoader.h"

#include "utils.h"

#include <fstream>
#include <algorithm>
#include <iterator>
#include <regex> 

using namespace DirectX;

Model::Model() {
	m_vertex_buffer = 0;
	m_index_buffer = 0;

	transform = Transform();

	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.SpecularPower = 20.f;
	//m_material.useTexture = false;
}
Model::Model(const Model& other) {

}
Model::~Model() {

}
void Model::shutdown() {
	if (m_index_buffer) {
		m_index_buffer->Release();
		m_index_buffer = 0;
	}
	if (m_vertex_buffer) {
		m_vertex_buffer->Release();
		m_vertex_buffer = 0;
	}

}
void Model::setActive(ID3D11DeviceContext* context) {
	unsigned int stride;
	unsigned int offset;

	stride = m_stride;
	offset = 0;

	context->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);
	context->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
int Model::getIndexCount() {
	return m_index_count;
}
/*bool Model::loadModelData(std::vector<XMFLOAT3>vertex_data, std::vector<XMFLOAT2>uv_data, std::vector<XMFLOAT3>normal_data, std::vector<XMINT3>index_data) {
	for (int i = 0; i < index_data.size(); i++) {
		VBO vt;
		vt.position = vertex_data[index_data[i].x];
		vt.uv = uv_data[index_data[i].y];
		vt.normal = normal_data[index_data[i].z];

		//doesnt work with models that have no uv data!
		//linear search should be replaced asap!
		bool is_in_list = false;
		for (int j = 0; j < vertices.size(); j++) {
			if (vt.position.x == vertices[j].position.x && 
				vt.position.y == vertices[j].position.y && 
				vt.position.z == vertices[j].position.z &&
				vt.normal.x == vertices[j].normal.x &&
				vt.normal.y == vertices[j].normal.y &&
				vt.normal.z == vertices[j].normal.z && 
				vt.uv.x == vertices[j].uv.x && 
				vt.uv.y == vertices[j].uv.y
				) {
				is_in_list = true;
				indices.push_back(j);
				break;
			}
		}
		if (!is_in_list) {
			indices.push_back(vertices.size());
			vertices.push_back(vt);
		}
	}

	m_vertex_count = vertices.size();
	m_index_count = indices.size();

	return true;
}*/

struct VBO {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 normal;
};

bool loadTrianglePrimitive(ID3D11Device* device, Model* m);
bool loadQuadPrimitive(ID3D11Device* device, Model* m);

bool loadModelFromFile(std::string file, ID3D11Device* device, Model* m) {
	std::vector<XMFLOAT3>vertex_data;
	std::vector<XMFLOAT2>uv_data;
	std::vector<XMFLOAT3>normal_data;
	std::vector<XMINT3>index_data;

	bool res;

	std::ifstream input(file);
	if (input.is_open()) {
		std::string line;
		while (std::getline(input, line)) {
			std::string marker = line.substr(0, 2);
			std::string payload = line.substr(2, line.size());
			if (marker == "v ") {
				XMFLOAT3 p;
				std::vector<std::string> tokens = splitString(payload, std::regex("[0-9.-]+"));
				if (tokens.size() != 3) {
					break;
				}
				p.x = std::stof(tokens[0]);
				p.y = std::stof(tokens[1]);
				p.z = std::stof(tokens[2]);

				vertex_data.push_back(p);
			}
			else if (marker == "vt") {
				XMFLOAT2 p;
				std::vector<std::string> tokens = splitString(payload, std::regex("[0-9.-]+"));
				if (tokens.size() != 2) {
					break;
				}
				p.x = std::stof(tokens[0]);
				p.y = std::stof(tokens[1]);

				uv_data.push_back(p);
			}
			else if (marker == "vn") {
				XMFLOAT3 p;
				std::vector<std::string> tokens = splitString(payload, std::regex("[0-9.-]+"));
				if (tokens.size() != 3) {
					break;
				}
				p.x = std::stof(tokens[0]);
				p.y = std::stof(tokens[1]);
				p.z = std::stof(tokens[2]);

				normal_data.push_back(p);
			}
			else if (marker == "f ") {
				std::vector<std::string> tokens = splitString(payload, std::regex("[0-9]+"));
				int step = 3;
				if (uv_data.size() == 0) {
					step = 2;
				}
				for (int i = 0; i < tokens.size(); i += step) {
					XMINT3 ind;
					if (uv_data.size() == 0) {
						ind.x = std::stoi(tokens[i]) - 1;
						ind.y = -1;
						ind.z = std::stoi(tokens[i + 1]) - 1;
					}
					else {
						ind.x = std::stoi(tokens[i]) - 1;
						ind.y = std::stoi(tokens[i + 1]) - 1;
						ind.z = std::stoi(tokens[i + 2]) - 1;
					}

					index_data.push_back(ind);
				}

			}
		}
	}else {
		return false;
	}

	std::vector<VBO> vertices;
	std::vector<unsigned long> indices;

	for (int i = 0; i < index_data.size(); i++) {
		VBO vt;
		vt.position = vertex_data[index_data[i].x];
		vt.uv = uv_data[index_data[i].y];
		vt.normal = normal_data[index_data[i].z];

		//doesnt work with models that have no uv data!
		//linear search should be replaced asap!
		bool is_in_list = false;
		for (int j = 0; j < vertices.size(); j++) {
			if (vt.position.x == vertices[j].position.x &&
				vt.position.y == vertices[j].position.y &&
				vt.position.z == vertices[j].position.z &&
				vt.normal.x == vertices[j].normal.x &&
				vt.normal.y == vertices[j].normal.y &&
				vt.normal.z == vertices[j].normal.z &&
				vt.uv.x == vertices[j].uv.x &&
				vt.uv.y == vertices[j].uv.y
				) {
				is_in_list = true;
				indices.push_back(j);
				break;
			}
		}
		if (!is_in_list) {
			indices.push_back(vertices.size());
			vertices.push_back(vt);
		}
	}

	res = m->init<VBO>(device, vertices, indices);
	if (!res) {
		return false;
	}
	return true;

}
bool loadPrimitive(int type, ID3D11Device* device, Model* m) {
	switch (type) {
		case PRIMITIVE_QUAD:		return loadQuadPrimitive(device, m);
		case PRIMITIVE_TRIANGLE:	return loadTrianglePrimitive(device, m);
		default: return false;
	}
}
bool loadQuadPrimitive(ID3D11Device* device, Model* m) {
	std::vector<XMFLOAT3>vertex_data;
	std::vector<XMFLOAT2>uv_data;
	std::vector<XMFLOAT3>normal_data;
	std::vector<XMINT3>index_data;

	bool res;

	vertex_data.push_back(XMFLOAT3(-1.0f, -1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(-1.0f, 1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(1.0f, -1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));

	uv_data.push_back(XMFLOAT2(0.f, 0.f));
	uv_data.push_back(XMFLOAT2(0.f, 50.f));
	uv_data.push_back(XMFLOAT2(50.f, 0.f));
	uv_data.push_back(XMFLOAT2(50.f, 50.f));
	
	normal_data.push_back(XMFLOAT3(0.0f, 0.0f, -1.0f));

	index_data.push_back(XMINT3(0, 0, 0));
	index_data.push_back(XMINT3(1, 1, 0));
	index_data.push_back(XMINT3(2, 2, 0));

	index_data.push_back(XMINT3(1, 1, 0));
	index_data.push_back(XMINT3(3, 3, 0));
	index_data.push_back(XMINT3(2, 2, 0));

	std::vector<VBO> vertices;
	std::vector<unsigned long> indices;

	for (int i = 0; i < index_data.size(); i++) {
		VBO vt;
		vt.position = vertex_data[index_data[i].x];
		vt.uv = uv_data[index_data[i].y];
		vt.normal = normal_data[index_data[i].z];

		//doesnt work with models that have no uv data!
		//linear search should be replaced asap!
		bool is_in_list = false;
		for (int j = 0; j < vertices.size(); j++) {
			if (vt.position.x == vertices[j].position.x &&
				vt.position.y == vertices[j].position.y &&
				vt.position.z == vertices[j].position.z &&
				vt.normal.x == vertices[j].normal.x &&
				vt.normal.y == vertices[j].normal.y &&
				vt.normal.z == vertices[j].normal.z &&
				vt.uv.x == vertices[j].uv.x &&
				vt.uv.y == vertices[j].uv.y
				) {
				is_in_list = true;
				indices.push_back(j);
				break;
			}
		}
		if (!is_in_list) {
			indices.push_back(vertices.size());
			vertices.push_back(vt);
		}
	}

	res = m->init<VBO>(device, vertices, indices);
	if (!res) {
		return false;
	}
	return true;
}
bool loadTrianglePrimitive(ID3D11Device* device, Model* m) {
	std::vector<XMFLOAT3>vertex_data;
	std::vector<XMFLOAT2>uv_data;
	std::vector<XMFLOAT3>normal_data;
	std::vector<XMINT3>index_data;

	bool res;

	vertex_data.push_back(XMFLOAT3(-1.0f, -1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(-1.0f, 1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(1.0f, -1.0f, 0.0f));
	vertex_data.push_back(XMFLOAT3(1.0f, 1.0f, 0.0f));

	normal_data.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	normal_data.push_back(XMFLOAT3(1.0f, 0.0f, 0.0f));

	index_data.push_back(XMINT3(0, -1, 0));
	index_data.push_back(XMINT3(1, -1, 1));
	index_data.push_back(XMINT3(2, -1, 0));

	index_data.push_back(XMINT3(1, -1, 1));
	index_data.push_back(XMINT3(3, -1, 1));
	index_data.push_back(XMINT3(2, -1, 0));

	std::vector<VBO> vertices;
	std::vector<unsigned long> indices;

	res = m->init<VBO>(device, vertices, indices);
	if (!res) {
		return false;
	}
	return true;
}
bool loadRect(ID3D11Device* device, float width, float height, bool use_normals, Model* m) {
	std::vector<XMFLOAT3>vertex_data;
	std::vector<XMFLOAT2>uv_data;

	std::vector<VBO> vertices;
	std::vector<unsigned long> indices;

	if (!m->init<VBO>(device, vertices, indices)) {
		return false;
	}
	return true;
}