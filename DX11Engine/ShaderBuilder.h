#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "Shader.h"
#include "Renderer.h"

class ShaderBuilder {
	public:
		ShaderBuilder();
		ShaderBuilder(const ShaderBuilder&);
		~ShaderBuilder();
		
		bool build(Renderer*, Shader*); // build and set shader, return true if succesful
		bool isBuildable();
		void clear(); // clear the stored values, called automatically when build is called

		//add components...;
		bool createVertexShader(WCHAR*, const char*, const char*, UINT, UINT);
		bool createPixelShader(WCHAR*, const char*, const char*, UINT, UINT);
		bool createGeometryShader(WCHAR*, const char*, const char*, UINT, UINT);

		void addElementToLayout(LPCSTR, UINT, DXGI_FORMAT, UINT, UINT, D3D11_INPUT_CLASSIFICATION, UINT);

		bool createSampler(std::string);
		bool setSamplerDesc(std::string, D3D11_FILTER, D3D11_TEXTURE_ADDRESS_MODE, D3D11_TEXTURE_ADDRESS_MODE, D3D11_TEXTURE_ADDRESS_MODE, FLOAT, UINT, D3D11_COMPARISON_FUNC, DirectX::XMFLOAT4, FLOAT, FLOAT);
		
		bool createBuffer(std::string, unsigned int, SHADER_TYPES);
		bool setBufferDesc(std::string, D3D11_USAGE, UINT, UINT, UINT, UINT, UINT);

	private:
		void outputShaderErrorMsg(ID3D10Blob*, WCHAR*);

		ID3D10Blob* m_error_msg;

		//output to shader
		std::vector<D3D11_INPUT_ELEMENT_DESC> m_layout;
		std::map<std::string, ShaderBufferDesc> m_buffers;
		std::map<std::string, D3D11_SAMPLER_DESC> m_samplers;
		ID3D10Blob* m_vertex_buffer;
		ID3D10Blob* m_pixel_buffer;
		ID3D10Blob* m_geo_buffer;

		bool encountered_error;
};

#endif
