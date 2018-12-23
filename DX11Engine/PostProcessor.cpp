#include "PostProcessor.h"

PostProcessor::PostProcessor() {
	m_scale = 0.5f;

	m_render_tex = 0;
	m_downscale_tex = 0;
	m_upscale_tex = 0;
	m_final_tex = 0;
}
PostProcessor::~PostProcessor() {

}
bool PostProcessor::init(Renderer* renderer, Shader* basic, Shader* proc) {
	m_render_tex = new RenderTexture();
	bool res = m_render_tex->init(renderer->getDevice(), renderer->getViewportWidth(), renderer->getViewportHeight(), SCREEN_NEAR, SCREEN_DEPTH);
	if (!res) {
		return false;
	}

	m_downscale_tex = new RenderTexture();
	res = m_downscale_tex->init(renderer->getDevice(), renderer->getViewportWidth() * m_scale, renderer->getViewportHeight() * m_scale, SCREEN_NEAR, SCREEN_DEPTH);
	if (!res) {
		return false;
	}

	m_upscale_tex = new RenderTexture();
	res = m_upscale_tex->init(renderer->getDevice(), renderer->getViewportWidth(), renderer->getViewportHeight(), SCREEN_NEAR, SCREEN_DEPTH);
	if (!res) {
		return false;
	}

	m_final_tex = new RenderTexture();
	res = m_final_tex->init(renderer->getDevice(), renderer->getViewportWidth(), renderer->getViewportHeight(), SCREEN_NEAR, SCREEN_DEPTH);
	if (!res) {
		return false;
	}

	m_small_window = new Rect();
	if (!m_small_window->init(renderer->getDevice(), renderer->getViewportWidth() * m_scale, renderer->getViewportHeight() * m_scale)) {
		return false;
	}

	m_large_window = new Rect();
	if (!m_large_window->init(renderer->getDevice(), renderer->getViewportWidth(), renderer->getViewportHeight())) {
		return false;
	}

	ShaderBuilder sb;
	sb.createVertexShader(L"res/shaders/Texture.vs", "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);
	sb.createPixelShader(L"res/shaders/Texture.ps", "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);

	sb.addElementToLayout("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	sb.addElementToLayout("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);

	sb.createSampler("ss");
	sb.setSamplerDesc("ss", D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_WRAP, 0.0f, 1, D3D11_COMPARISON_ALWAYS, XMFLOAT4(0.f, 0.f, 0.f, 0.f), 0, D3D11_FLOAT32_MAX);

	sb.createBuffer("matrix_buffer", 0, VERTEX_SHADER);
	sb.setBufferDesc("matrix_buffer", D3D11_USAGE_DYNAMIC, sizeof(MatrixBuffer), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0);

	m_basic_shader = new Shader();
	if (!sb.build(renderer, m_basic_shader)) {
		return false;
	}

	return true;
}
void PostProcessor::shutdown() {
	if (m_render_tex) {
		m_render_tex->shutdown();
		delete m_render_tex;
		m_render_tex = 0;
	}
	if (m_upscale_tex) {
		m_upscale_tex->shutdown();
		delete m_upscale_tex;
		m_upscale_tex = 0;
	}
	if (m_downscale_tex) {
		m_downscale_tex->shutdown();
		delete m_downscale_tex;
		m_downscale_tex = 0;
	}
	if (m_final_tex) {
		m_final_tex->shutdown();
		delete m_final_tex;
		m_final_tex = 0;
	}
	if (m_small_window) {
		m_small_window->shutdown();
		delete m_small_window;
		m_small_window = 0;
	}
	if (m_large_window) {
		m_large_window->shutdown();
		delete m_large_window;
		m_large_window = 0;
	}
}
void PostProcessor::setActiveCamera(Camera* camera) {
	m_cam = camera;
}
bool PostProcessor::process(Renderer* renderer) {

	if (!downScale(renderer)) {
		return false;
	}

	if (!upScale(renderer)) {
		return false;
	}

	if (!renderFinal(renderer)) {
		return false;
	}

	return true;
}
bool PostProcessor::upScale(Renderer* renderer) {
	return true;
}
bool PostProcessor::downScale(Renderer* renderer) {
	return true;
}
bool PostProcessor::renderFinal(Renderer* renderer) {

	renderer->setUseZBuffer(false);

	MatrixBuffer mb;
	mb.world = renderer->getWorldMatrix();
	mb.view = renderer->getViewMatrix();
	mb.projection = renderer->getOrthoProjectionMatrix();

	m_basic_shader->setResource(PIXEL_SHADER, renderer->getContext(), m_render_tex->GetShaderResourceView());
	m_basic_shader->setParams<MatrixBuffer>(renderer->getContext(), "matrix_buffer", mb);
	m_basic_shader->setActive(renderer->getContext());

	m_large_window->render(renderer);

	renderer->setUseZBuffer(true);

	return true;
}
void PostProcessor::clearRenderTexture(Renderer* renderer) {
	
	m_render_tex->clear(renderer->getContext(), renderer->getClearColor().r, renderer->getClearColor().g, renderer->getClearColor().b);
	//m_render_tex->clear(renderer->getContext(), 0, 0, 0);
	//m_render_tex->clear(renderer->getContext(), 0, 0, 0);
	//m_render_tex->clear(renderer->getContext(), 0, 0, 0);
}