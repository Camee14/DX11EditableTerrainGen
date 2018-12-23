#ifndef SHADERTEMPLATE_H
#define SHADERTEMPLATE_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include <fstream>
#include <map>
#include <vector>
#include <string>

const enum SHADER_TYPES {
	VERTEX_SHADER,
	PIXEL_SHADER,
	GEOMETRY_SHADER
};
struct ShaderBufferDesc {
	D3D11_BUFFER_DESC desc;
	unsigned int index;
	SHADER_TYPES type;
};

class Shader {
public:
	Shader();
	Shader(const Shader&);
	~Shader();

	bool init(ID3D11Device*, ID3D10Blob*, ID3D10Blob*, std::map<std::string, ShaderBufferDesc>, std::map<std::string, D3D11_SAMPLER_DESC>, std::vector<D3D11_INPUT_ELEMENT_DESC>);
	bool init(ID3D11Device*, ID3D10Blob*, ID3D10Blob*, ID3D10Blob*, std::map<std::string, ShaderBufferDesc>, std::map<std::string, D3D11_SAMPLER_DESC>, std::vector<D3D11_INPUT_ELEMENT_DESC>);

	void shutdown();
	void setActive(ID3D11DeviceContext*);

	bool setResource(SHADER_TYPES, ID3D11DeviceContext*, ID3D11ShaderResourceView*);
	bool setSampler(std::string);

	template<typename T>
	bool setParams(ID3D11DeviceContext* context, std::string key, T buffer) {
		if (m_buffers.find(key) == m_buffers.end()) {
			return false;
		}
		if (m_buffers.at(key).ptr) {
			HRESULT res;
			D3D11_MAPPED_SUBRESOURCE mapped_resource;

			res = context->Map(m_buffers.at(key).ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
			if (FAILED(res)) {
				return false;
			}

			T* ptr = (T*)mapped_resource.pData;
			memcpy(ptr, &buffer, sizeof(buffer));

			context->Unmap(m_buffers.at(key).ptr, 0);

			if (m_buffers.at(key).type == VERTEX_SHADER) {
				context->VSSetConstantBuffers(m_buffers.at(key).index, 1, &m_buffers.at(key).ptr);
			}
			else if (m_buffers.at(key).type == PIXEL_SHADER) {
				context->PSSetConstantBuffers(m_buffers.at(key).index, 1, &m_buffers.at(key).ptr);
			}
			else if (m_buffers.at(key).type == GEOMETRY_SHADER) {
				context->GSSetConstantBuffers(m_buffers.at(key).index, 1, &m_buffers.at(key).ptr);
			}

			return true;
		}
		else {
			return false;
		}
	}

private:
	struct ShaderBuffer {
		ID3D11Buffer* ptr;
		unsigned int index;
		SHADER_TYPES type;

	};
	struct ShaderSampler {
		ID3D11SamplerState* ptr;
	};

	bool finishInit(ID3D11Device*, std::map<std::string, ShaderBufferDesc>, std::map<std::string, D3D11_SAMPLER_DESC>);

	void outputShaderErrorMsg(ID3D10Blob*, HWND, WCHAR*);
	void outputErrorMsg(HRESULT);

	ID3D11VertexShader* m_vertex_shader;
	ID3D11PixelShader* m_pixel_shader;
	ID3D11GeometryShader* m_geo_shader;

	ID3D11InputLayout* m_layout;

	std::string m_active_sampler;

	std::map<std::string, ShaderBuffer> m_buffers;
	std::map<std::string, ShaderSampler> m_samplers;

};

#endif
