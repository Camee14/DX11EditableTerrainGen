#include "ShaderBuilder.h"

using namespace DirectX;

ShaderBuilder::ShaderBuilder() {
	encountered_error = false;
}
ShaderBuilder::ShaderBuilder(const ShaderBuilder& other) {

}
ShaderBuilder::~ShaderBuilder() {

}
bool ShaderBuilder::isBuildable() {
	return !encountered_error;
}
bool ShaderBuilder::build(Renderer* r, Shader* s) {
	if (!isBuildable()) {
		clear();

		return false;
	}

	bool res;
	if (m_geo_buffer) {
		res = s->init(r->getDevice(), m_vertex_buffer, m_pixel_buffer, m_geo_buffer, m_buffers, m_samplers, m_layout);
	}
	else {
		res = s->init(r->getDevice(), m_vertex_buffer, m_pixel_buffer, m_buffers, m_samplers, m_layout);
	}
	
	clear();

	return res;
}
void ShaderBuilder::clear() {
	if (m_vertex_buffer) {
		m_vertex_buffer->Release();
		m_vertex_buffer = 0;
	}
	if (m_pixel_buffer) {
		m_pixel_buffer->Release();
		m_pixel_buffer = 0;
	}
	if (m_geo_buffer) {
		m_geo_buffer->Release();
		m_geo_buffer = 0;
	}
	m_layout.clear();
	m_buffers.clear();
	m_samplers.clear();
}
bool ShaderBuilder::createVertexShader(WCHAR* path, const char* entry, const char* target, UINT flags1, UINT flags2) {
	HRESULT res;

	res = D3DCompileFromFile(path, NULL, NULL, entry, target, flags1, flags2, &m_vertex_buffer, &m_error_msg);
	if (FAILED(res)) {
		if (m_error_msg) {
			outputShaderErrorMsg(m_error_msg, path);
		}
		else {
			printf("cound not find vertex shader file: %s \n", path);
		}
		encountered_error = true;
		return false;
	}
	return true;
}
bool ShaderBuilder::createPixelShader(WCHAR* path, const char* entry, const char* target, UINT flags1, UINT flags2) {
	HRESULT res;

	res = D3DCompileFromFile(path, NULL, NULL, entry, target, flags1, flags2, &m_pixel_buffer, &m_error_msg);
	if (FAILED(res)) {
		if (m_error_msg) {
			outputShaderErrorMsg(m_error_msg, path);
		}
		else {
			printf("cound not find pixel shader file: %s \n", path);
		}
		encountered_error = true;
		return false;
	}
	return true;
}
bool ShaderBuilder::createGeometryShader(WCHAR* path, const char* entry, const char* target, UINT flags1, UINT flags2) {
	HRESULT res;

	res = D3DCompileFromFile(path, NULL, NULL, entry, target, flags1, flags2, &m_geo_buffer, &m_error_msg);
	if (FAILED(res)) {
		if (m_error_msg) {
			outputShaderErrorMsg(m_error_msg, path);
		}
		else {
			printf("cound not find pixel shader file: %s \n", path);
		}
		encountered_error = true;
		return false;
	}
	return true;
}
void ShaderBuilder::addElementToLayout(LPCSTR name, UINT index, DXGI_FORMAT format, UINT input_slot, UINT offset, D3D11_INPUT_CLASSIFICATION input_class, UINT step_rate) {
	D3D11_INPUT_ELEMENT_DESC e;
	e.SemanticName = name;
	e.SemanticIndex = index;
	e.Format = format;
	e.InputSlot = input_slot;
	e.AlignedByteOffset = offset;
	e.InputSlotClass = input_class;
	e.InstanceDataStepRate = step_rate;

	m_layout.push_back(e);

}
bool ShaderBuilder::createSampler(std::string key) {
	if (m_samplers.find(key) != m_samplers.end()) {
		encountered_error = true;
		return false;
	}
	D3D11_SAMPLER_DESC sd;
	m_samplers.emplace(key, sd);

	return true;
}
bool ShaderBuilder::setSamplerDesc(std::string key, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE u, D3D11_TEXTURE_ADDRESS_MODE v, D3D11_TEXTURE_ADDRESS_MODE w, FLOAT bias, UINT aniso, D3D11_COMPARISON_FUNC comp, XMFLOAT4 colour , FLOAT min_lod, FLOAT max_lod) {
	if (m_samplers.find(key) == m_samplers.end()) {
		encountered_error = true;
		return false;
	}

	m_samplers.at(key).Filter = filter;
	m_samplers.at(key).AddressU = u;
	m_samplers.at(key).AddressV = v;
	m_samplers.at(key).AddressW = w;
	m_samplers.at(key).MipLODBias = bias;
	m_samplers.at(key).MaxAnisotropy = aniso;
	m_samplers.at(key).ComparisonFunc = comp;
	m_samplers.at(key).BorderColor[0] = colour.x;
	m_samplers.at(key).BorderColor[1] = colour.y;
	m_samplers.at(key).BorderColor[2] = colour.z;
	m_samplers.at(key).BorderColor[3] = colour.w;
	m_samplers.at(key).MinLOD = min_lod;
	m_samplers.at(key).MaxLOD = max_lod;

	return true;
}
bool ShaderBuilder::createBuffer(std::string key, unsigned int index, SHADER_TYPES type) {
	if (m_buffers.find(key) != m_buffers.end()) {
		encountered_error = true;
		return false;
	}
	ShaderBufferDesc sbd;
	sbd.index = index;
	sbd.type = type;

	m_buffers.emplace(key, sbd);

	return true;
}
bool ShaderBuilder::setBufferDesc(std::string key, D3D11_USAGE usage, UINT byte_width, UINT bind_flags, UINT cpu_flags, UINT misc_flags, UINT stride) {
	if (m_buffers.find(key) == m_buffers.end()) {
		encountered_error = true;
		return false;
	}

	m_buffers.at(key).desc.Usage = usage;
	m_buffers.at(key).desc.ByteWidth = byte_width;
	m_buffers.at(key).desc.BindFlags = bind_flags;
	m_buffers.at(key).desc.CPUAccessFlags = cpu_flags;
	m_buffers.at(key).desc.MiscFlags = misc_flags;
	m_buffers.at(key).desc.StructureByteStride = stride;

	return true;
}
void ShaderBuilder::outputShaderErrorMsg(ID3D10Blob* msg, WCHAR* file) {
	char* compile_errors;
	unsigned long long buffer_size, i;
	std::ofstream fout;

	compile_errors = (char*)(msg->GetBufferPointer());

	buffer_size = msg->GetBufferSize();

	fout.open("shader_error.txt");

	for (i = 0; i < buffer_size; i++) {
		fout << compile_errors[i];
	}
	fout.close();

	msg->Release();
	msg = 0;

	printf("error compiling shader, error message exported to shader_error.txt \n");
}