#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include <d3d11.h>
#include <directxmath.h>

class RenderTexture {
public:
	RenderTexture();
	~RenderTexture();

	bool init(ID3D11Device*, int, int, float, float);
	void shutdown();
	void clear(ID3D11DeviceContext*, float, float, float);

	ID3D11ShaderResourceView* GetShaderResourceView() const {
		return m_shader_res_view;
	}
	ID3D11RenderTargetView* getView() const {
		return m_target_view;
	}
	ID3D11DepthStencilView* getDepthStencilView() const{
		return m_depth_stencil_view;
	}
	D3D11_VIEWPORT getViewport() const {
		return m_viewport;
	}
	DirectX::XMMATRIX getOrthographicMatrix() const {
		return m_ortho_mat;
	}
private:
	int m_width, m_height;
	ID3D11Texture2D* m_texture;
	ID3D11Texture2D* m_depth;
	ID3D11RenderTargetView* m_target_view;
	ID3D11DepthStencilView* m_depth_stencil_view;
	ID3D11ShaderResourceView* m_shader_res_view;
	D3D11_VIEWPORT m_viewport;

	DirectX::XMMATRIX m_proj_mat;
	DirectX::XMMATRIX m_ortho_mat;
};

#endif
