#include "Direct3D.h"

Direct3D::Direct3D() {
	swap_chain = 0;
	device = 0;
	device_context = 0;
	render_target_view = 0;
	depth_stencil_buffer = 0;
	depth_stencil_view = 0;
	raster_state = 0;

	m_alpha_blend_state_enable = 0;
	m_alpha_blend_state_disable = 0;

	depth_stencil_state_enabled = 0;
	depth_stencil_state_disabled = 0;
}
Direct3D::Direct3D(const Direct3D& other) {

}
Direct3D::~Direct3D() {

}
bool Direct3D::init(int screen_width, int screen_height, bool vsync, HWND hwnd, bool fullscreen, float screen_near, float screen_depth) {
	HRESULT res;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapter_out;
	unsigned int num_modes, i, numerator, denominator;
	unsigned long long string_length;
	DXGI_MODE_DESC* display_mode_list;
	DXGI_ADAPTER_DESC adapter_desc;
	int error;
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	D3D_FEATURE_LEVEL feature_level;
	ID3D11Texture2D* back_buffer_ptr;
	D3D11_TEXTURE2D_DESC depth_buffer_desc;
	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	D3D11_RASTERIZER_DESC raster_desc;
	D3D11_BLEND_DESC blend_state_desc;
	float FOV, aspect_ratio;

	v_sync = vsync;

	res = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(res)) {
		return false;
	}

	res = factory->EnumAdapters(0, &adapter);
	if (FAILED(res)) {
		return false;
	}

	res = adapter->EnumOutputs(0, &adapter_out); 
	if(FAILED(res)){
		return false;
	}

	res = adapter_out->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, NULL);
	if (FAILED(res)) {
		return false;
	}

	display_mode_list = new DXGI_MODE_DESC[num_modes];
	if (!display_mode_list) {
		return false;
	}

	res = adapter_out->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, display_mode_list);
	if (FAILED(res)) {
		return false;
	}

	for (int i = 0; i < num_modes; i++) {
		if (display_mode_list[i].Width == (unsigned int)screen_width && display_mode_list[i].Height == (unsigned int)screen_height) {
			numerator = display_mode_list[i].RefreshRate.Numerator;
			denominator = display_mode_list[i].RefreshRate.Denominator;
		}
	}

	res = adapter->GetDesc(&adapter_desc);
	if(FAILED(res)){
		return false;
	}
	video_card_mem = (int)(adapter_desc.DedicatedVideoMemory / 1024 / 1024);

	error = wcstombs_s(&string_length, video_card_desc, 128, adapter_desc.Description, 128);
	if (error != 0) {
		return false;
	}

	delete[] display_mode_list;
	display_mode_list = 0;

	adapter_out->Release();
	adapter_out = 0;

	adapter->Release();
	adapter = 0;

	factory->Release();
	factory = 0;

	//SWAP CHAIN DESCRIPTION

	ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferDesc.Width = screen_width;
	swap_chain_desc.BufferDesc.Height = screen_height;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (v_sync) {
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
	}else {
		swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;

	if (fullscreen) {
		swap_chain_desc.Windowed = false;
	}else {
		swap_chain_desc.Windowed = true;
	}

	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;

	feature_level = D3D_FEATURE_LEVEL_11_0;

	res = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&feature_level,
		1,
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		&swap_chain,
		&device,
		NULL,
		&device_context
	);
	if (FAILED(res)) {
		return false;
	}

	res = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer_ptr);
	if (FAILED(res)) {
		return false;
	}

	res = device->CreateRenderTargetView(back_buffer_ptr, NULL, &render_target_view);
	if (FAILED(res)) {
		return false;
	}
	back_buffer_ptr->Release();
	back_buffer_ptr = 0;

	//DEPTH BUFFER DESCRIPTION
	ZeroMemory(&depth_buffer_desc, sizeof(depth_buffer_desc));
	depth_buffer_desc.Width = screen_width;
	depth_buffer_desc.Height = screen_height;
	depth_buffer_desc.MipLevels = 1;
	depth_buffer_desc.ArraySize = 1;
	depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_buffer_desc.SampleDesc.Count = 1;
	depth_buffer_desc.SampleDesc.Quality = 0;
	depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_buffer_desc.CPUAccessFlags = 0;
	depth_buffer_desc.MiscFlags = 0;

	res = device->CreateTexture2D(&depth_buffer_desc, NULL, &depth_stencil_buffer);
	if (FAILED(res)) {
		return false;
	}

	//DEPTH STENCIL DESCRIPTION
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = true;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

	depth_stencil_desc.StencilEnable = true;
	depth_stencil_desc.StencilReadMask = 0xFF;
	depth_stencil_desc.StencilWriteMask = 0xFF;

	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	res = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_enabled);
	if (FAILED(res)) {
		return false;
	}
	device_context->OMSetDepthStencilState(depth_stencil_state_enabled, 1);

	//DEPTH STENCIL DESCRIPTION
	ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));
	depth_stencil_desc.DepthEnable = false;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

	depth_stencil_desc.StencilEnable = true;
	depth_stencil_desc.StencilReadMask = 0xFF;
	depth_stencil_desc.StencilWriteMask = 0xFF;

	depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	res = device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state_disabled);
	if (FAILED(res)) {
		return false;
	}

	//DEPTH STENCIL BUFFER VIEW DESCRIPTION
	ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));
	depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Texture2D.MipSlice = 0;

	res = device->CreateDepthStencilView(depth_stencil_buffer, &depth_stencil_view_desc, &depth_stencil_view);
	if (FAILED(res)) {
		return false;
	}

	device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);

	//RASTERIZER DESCRIPTION
	raster_desc.AntialiasedLineEnable = false;
	raster_desc.CullMode = D3D11_CULL_BACK;
	raster_desc.DepthBias = 0;
	raster_desc.DepthBiasClamp = 0.0f;
	raster_desc.DepthClipEnable = true;
	raster_desc.FillMode = D3D11_FILL_SOLID;
	raster_desc.FrontCounterClockwise = false;
	raster_desc.MultisampleEnable = false;
	raster_desc.ScissorEnable = false;
	raster_desc.SlopeScaledDepthBias = 0.0f;

	res = device->CreateRasterizerState(&raster_desc, &raster_state);
	if (FAILED(res)) {
		return false;
	}
	device_context->RSSetState(raster_state);

	//VIEWPORT
	viewport.Width = (float)screen_width;
	viewport.Height = (float)screen_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	device_context->RSSetViewports(1, &viewport);

	//PROJECTION MATRIX
	FOV = 3.141592654f / 4.0f;
	aspect_ratio = (float)screen_width / (float)screen_height;

	projection_matrix = XMMatrixPerspectiveFovLH(FOV, aspect_ratio, screen_near, screen_depth);
	world_matrix = XMMatrixIdentity();
	ortho_matrix = XMMatrixOrthographicLH((float)screen_width, (float)screen_height, screen_near, screen_depth);

	ZeroMemory(&blend_state_desc, sizeof(D3D11_BLEND_DESC));

	blend_state_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_state_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_state_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_state_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_state_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_state_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_state_desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	res = device->CreateBlendState(&blend_state_desc, &m_alpha_blend_state_enable);
	if (FAILED(res))
	{
		return false;
	}

	blend_state_desc.RenderTarget[0].BlendEnable = FALSE;

	res = device->CreateBlendState(&blend_state_desc, &m_alpha_blend_state_disable);
	if (FAILED(res))
	{
		return false;
	}

	return true;
}
void Direct3D::shutdown() {
	if (swap_chain) {
		swap_chain->SetFullscreenState(false, NULL);
	}
	if (m_alpha_blend_state_enable)
	{
		m_alpha_blend_state_enable->Release();
		m_alpha_blend_state_enable = 0;
	}

	if (m_alpha_blend_state_disable)
	{
		m_alpha_blend_state_disable->Release();
		m_alpha_blend_state_disable = 0;
	}
	if (raster_state) {
		raster_state->Release();
		raster_state = 0;
	}
	if (depth_stencil_view) {
		depth_stencil_view->Release();
		depth_stencil_view = 0;
	}
	if (depth_stencil_state_enabled) {
		depth_stencil_state_enabled->Release();
		depth_stencil_state_enabled = 0;
	}
	if (depth_stencil_buffer) {
		depth_stencil_buffer->Release();
		depth_stencil_buffer = 0;
	}
	if (render_target_view) {
		render_target_view->Release();
		render_target_view = 0;
	}
	if (device_context) {
		device_context->Release();
		device_context = 0;
	}
	if (device) {
		device->Release();
		device = 0;
	}
	if (swap_chain) {
		swap_chain->Release();
		swap_chain = 0;
	}

}
void Direct3D::beginScene(float r, float g, float b, float a) {
	float colour[4];

	colour[0] = r;
	colour[1] = g;
	colour[2] = b;
	colour[3] = a;

	device_context->ClearRenderTargetView(render_target_view, colour);
	device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Direct3D::endScene() {
	if (v_sync) {
		swap_chain->Present(1, 0);
	}else {
		swap_chain->Present(0, 0);
	}
}
ID3D11Device* Direct3D::getDevice() {
	return device;
}
ID3D11DeviceContext* Direct3D::getDeviceContext() {
	return device_context;
}
void Direct3D::getProjectionMatrix(XMMATRIX& mat) {
	mat = projection_matrix;
}
void Direct3D::getWorldMatrix(XMMATRIX& mat) {
	mat = world_matrix;
}
void Direct3D::getOrthoMatrix(XMMATRIX& mat) {
	mat = ortho_matrix;
}
float Direct3D::getDefaultViewportWidth(){
	return viewport.Width;
}
float Direct3D::getDefaultViewportHeight() {
	return viewport.Height;
}
void Direct3D::getVideoCardInfo(char* name, int& memory) {
	strcpy_s(name, 128, video_card_desc);
	memory = video_card_mem;
}
void Direct3D::alphaBlending(bool enabled) {
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	if (enabled) {
		// Turn on the alpha blending.
		device_context->OMSetBlendState(m_alpha_blend_state_enable, blendFactor, 0xffffffff);
	}
	else {
		// Turn off the alpha blending.
		device_context->OMSetBlendState(m_alpha_blend_state_disable, blendFactor, 0xffffffff);

	}
}
void Direct3D::zBuffer(bool enabled) {
	if (enabled) {
		device_context->OMSetDepthStencilState(depth_stencil_state_enabled, 1);
	}
	else {
		device_context->OMSetDepthStencilState(depth_stencil_state_disabled, 1);
	}
}
void Direct3D::defaultRenderTarget() {
	device_context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
}
void  Direct3D::defaultViewport() {
	device_context->RSSetViewports(1, &viewport);

}