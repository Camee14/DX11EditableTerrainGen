#include "RenderTexture.h"

using namespace DirectX;

RenderTexture::RenderTexture() {
	m_texture = 0;
	m_target_view = 0;
	m_depth = 0;
	m_depth_stencil_view = 0;
	m_shader_res_view = 0;
}
RenderTexture::~RenderTexture() {

}
bool RenderTexture::init(ID3D11Device* device, int width, int height, float screen_near, float screen_far) {
	HRESULT res;
	D3D11_TEXTURE2D_DESC tex_desc, depth_desc;
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_res_view_desc;


	m_width = width;
	m_height = height;

	ZeroMemory(&tex_desc, sizeof(tex_desc));

	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;

	res = device->CreateTexture2D(&tex_desc, NULL, &m_texture);
	if (FAILED(res)) {
		return false;
	}

	render_target_view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	render_target_view_desc.Texture2D.MipSlice = 0;

	res = device->CreateRenderTargetView(m_texture, &render_target_view_desc, &m_target_view);
	if (FAILED(res)) {
		return false;
	}

	shader_res_view_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shader_res_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_res_view_desc.Texture2D.MostDetailedMip = 0;
	shader_res_view_desc.Texture2D.MipLevels = 1;

	res = device->CreateShaderResourceView(m_texture, &shader_res_view_desc, &m_shader_res_view);
	if (FAILED(res)) {
		return false;
	}

	ZeroMemory(&depth_desc, sizeof(depth_desc));

	depth_desc.Width = width;
	depth_desc.Height = height;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.SampleDesc.Quality = 0;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_desc.CPUAccessFlags = 0;
	depth_desc.MiscFlags = 0;

	res = device->CreateTexture2D(&depth_desc, NULL, &m_depth);
	if (FAILED(res)) {
		return false;
	}

	ZeroMemory(&depth_view_desc, sizeof(depth_view_desc));

	depth_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_view_desc.Texture2D.MipSlice = 0;

	res = device->CreateDepthStencilView(m_depth, &depth_view_desc, &m_depth_stencil_view);
	if(FAILED(res)){
		return false;
	}

	m_viewport.Width = (float)width;
	m_viewport.Height = (float)height;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	float FOV = 3.141592654f / 4.0f;
	m_proj_mat = XMMatrixPerspectiveFovLH(FOV, (float)width / (float)height, screen_near, screen_far);

	m_ortho_mat = XMMatrixOrthographicLH((float)width, (float)height, screen_near, screen_far);

	return true;
}
void RenderTexture::shutdown() {
	if (m_depth_stencil_view) {
		m_depth_stencil_view->Release();
		m_depth_stencil_view = 0;
	}
	if (m_depth) {
		m_depth->Release();
		m_depth = 0;
	}
	if (m_shader_res_view) {
		m_shader_res_view->Release();
		m_shader_res_view = 0;
	}
	if (m_target_view) {
		m_texture->Release();
		m_texture = 0;
	}
	if (m_texture) {
		m_texture->Release();
		m_texture = 0;
	}
}
void RenderTexture::clear(ID3D11DeviceContext* context, float r, float g, float b) {
	float colour[4];
	colour[0] = r;
	colour[1] = g;
	colour[2] = b;
	colour[3] = 1;

	context->ClearRenderTargetView(m_target_view, colour);
	context->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}