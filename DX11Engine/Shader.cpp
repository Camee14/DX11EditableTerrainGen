#include "Shader.h"

#include <stdio.h>
#include <windows.h>
#include <tchar.h>

Shader::Shader() {
	m_vertex_shader = 0;
	m_pixel_shader = 0;
	m_geo_shader = 0;

}
Shader::Shader(const Shader& other) {

}
Shader::~Shader() {

}
bool Shader::init(ID3D11Device* device, ID3D10Blob* vertex, ID3D10Blob* pixel, std::map<std::string, ShaderBufferDesc> buffer_descs, std::map<std::string, D3D11_SAMPLER_DESC> sampler_descs, std::vector<D3D11_INPUT_ELEMENT_DESC> layout) {
	HRESULT res;

	res = device->CreateVertexShader(vertex->GetBufferPointer(), vertex->GetBufferSize(), NULL, &m_vertex_shader);
	if (FAILED(res)) {
		return false;
	}

	res = device->CreatePixelShader(pixel->GetBufferPointer(), pixel->GetBufferSize(), NULL, &m_pixel_shader);
	if (FAILED(res)) {
		return false;
	}

	res = device->CreateInputLayout(&layout[0], layout.size(), vertex->GetBufferPointer(), vertex->GetBufferSize(), &m_layout);
	if (FAILED(res)) {
		outputErrorMsg(res);
		return false;
	}

	return finishInit(device, buffer_descs, sampler_descs);
}
bool Shader::init(ID3D11Device* device, ID3D10Blob* vertex, ID3D10Blob* pixel, ID3D10Blob* geo, std::map<std::string, ShaderBufferDesc> buffer_descs, std::map<std::string, D3D11_SAMPLER_DESC> sampler_descs, std::vector<D3D11_INPUT_ELEMENT_DESC> layout) {
	HRESULT res;

	res = device->CreateVertexShader(vertex->GetBufferPointer(), vertex->GetBufferSize(), NULL, &m_vertex_shader);
	if (FAILED(res)) {
		return false;
	}
	res = device->CreatePixelShader(pixel->GetBufferPointer(), pixel->GetBufferSize(), NULL, &m_pixel_shader);
	if (FAILED(res)) {
		return false;
	}
	res = device->CreateGeometryShader(geo->GetBufferPointer(), geo->GetBufferSize(), NULL, &m_geo_shader);
	if (FAILED(res)) {
		return false;
	}

	res = device->CreateInputLayout(&layout[0], layout.size(), vertex->GetBufferPointer(), vertex->GetBufferSize(), &m_layout);
	if (FAILED(res)) {
		outputErrorMsg(res);
		return false;
	}

	return finishInit(device, buffer_descs, sampler_descs);
}
bool Shader::finishInit(ID3D11Device* device, std::map<std::string, ShaderBufferDesc> buffer_descs, std::map<std::string, D3D11_SAMPLER_DESC> sampler_descs) {
	HRESULT res;
	for (auto const &sampler_desc : sampler_descs) {
		ShaderSampler sampler;
		res = device->CreateSamplerState(&sampler_desc.second, &sampler.ptr);
		if (FAILED(res)) {
			return false;
		}
		m_samplers.emplace(sampler_desc.first, sampler);
		m_active_sampler = sampler_desc.first;
	}

	for (auto const &buffer_desc : buffer_descs) {
		ShaderBuffer buffer;
		buffer.index = buffer_desc.second.index;
		buffer.type = buffer_desc.second.type;
		res = device->CreateBuffer(&buffer_desc.second.desc, NULL, &buffer.ptr);
		if (FAILED(res)) {
			return false;
		}
		m_buffers.emplace(buffer_desc.first, buffer);
	}
	return true;
}
void Shader::setActive(ID3D11DeviceContext* context) {
	context->IASetInputLayout(m_layout);
	context->VSSetShader(m_vertex_shader, NULL, 0);
	context->PSSetShader(m_pixel_shader, NULL, 0);

	context->GSSetShader(nullptr, NULL, 0);
	if (m_geo_shader) {
		context->GSSetShader(m_geo_shader, NULL, 0);
	}

	if (m_samplers.size() > 0) {
		context->PSSetSamplers(0, 1, &m_samplers[m_active_sampler].ptr);
	}
}
bool Shader::setResource(SHADER_TYPES type, ID3D11DeviceContext* context, ID3D11ShaderResourceView* resource) {
	switch (type) {
	case PIXEL_SHADER:	context->PSSetShaderResources(0, 1, &resource);
		break;
	default: return false;
	}
	return true;
}
bool Shader::setSampler(std::string key) {
	if (m_samplers.find(key) == m_samplers.end()) {
		return false;
	}
	m_active_sampler = key;
	return true;
}
void Shader::shutdown() {
	if (m_buffers.size() > 0) {
		for (auto const &buffer : m_buffers) {
			if (buffer.second.ptr) {
				buffer.second.ptr->Release();
			}
		}
	}
	if (m_samplers.size() > 0) {
		for (auto const &sampler : m_samplers) {
			if (sampler.second.ptr) {
				sampler.second.ptr->Release();
			}
		}
	}
	if (m_layout) {
		m_layout->Release();
		m_layout = 0;
	}
	if (m_pixel_shader) {
		m_pixel_shader->Release();
		m_pixel_shader = 0;
	}
	if (m_vertex_shader) {
		m_vertex_shader->Release();
		m_vertex_shader = 0;
	}
	if (m_geo_shader) {
		m_geo_shader->Release();
		m_geo_shader = 0;
	}
}
void Shader::outputShaderErrorMsg(ID3D10Blob* msg, HWND window, WCHAR* file) {
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

	MessageBox(window, "error compiling shader, error message exported to shader_error.txt", "file", MB_OK);
}
void Shader::outputErrorMsg(HRESULT res) {
	if (FACILITY_WINDOWS == HRESULT_FACILITY(res)) {
		res = HRESULT_CODE(res);
	}
	TCHAR* szErrMsg;

	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&szErrMsg, 0, NULL) != 0)
	{
		_tprintf(TEXT("%s"), szErrMsg);
		LocalFree(szErrMsg);
	}
	else {
		_tprintf(TEXT("[Could not find a description for error # %#x.]\n"), res);
	}
}