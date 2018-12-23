#include "Renderer.h"

Renderer::Renderer() {
	d3d = 0;
	active_camera = 0;
}
Renderer::Renderer(const Renderer& other) {

}
Renderer::~Renderer() {

}
bool Renderer::init(int screen_width, int screen_height, HWND hwnd) {
	bool res;
	d3d = new Direct3D;

	clear_colour.r = 118.0f / 255;
	clear_colour.g = 157.0f / 255;
	clear_colour.b = 219.0f / 255;
	clear_colour.a = 1.0f;

	if (!d3d) {
		return false;
	}
	res = d3d->init(screen_width, screen_height, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_NEAR, SCREEN_DEPTH);
	if (!res) {
		MessageBox(hwnd, "Could not initialize Direct3D", "Error", MB_OK);
		return false;
	}

	return true;
}
void Renderer::begin() {
	d3d->beginScene(clear_colour.r, clear_colour.g, clear_colour.b, clear_colour.a);
}
void Renderer::end() {
	d3d->endScene();
}
void Renderer::addLight(LightBuffer l) {
	m_lights.push_back(l);
}
LightBuffer* Renderer::getLights(int& num) {
	num = m_lights.size();

	return &m_lights[0];
}
void Renderer::setAlphaBlending(bool enabled) {
	d3d->alphaBlending(enabled);
}
void Renderer::setUseZBuffer(bool enabled) {
	d3d->zBuffer(enabled);
}
ID3D11Device* Renderer::getDevice() {
	return d3d->getDevice();
}
ID3D11DeviceContext* Renderer::getContext() {
	return d3d->getDeviceContext();
}
XMMATRIX Renderer::getWorldMatrix() {
	XMMATRIX m;
	d3d->getWorldMatrix(m);
	return m;
}
XMMATRIX Renderer::getViewMatrix() {
	XMMATRIX m;
	active_camera->getView(m);
	return m;
}
XMMATRIX Renderer::getProjectionMatrix() {
	XMMATRIX m;
	d3d->getProjectionMatrix(m);
	return m;
}
XMMATRIX Renderer::getOrthoProjectionMatrix() {
	XMMATRIX m;
	d3d->getOrthoMatrix(m);
	return m;
}
int Renderer::getViewportWidth() {
	return (int)d3d->getDefaultViewportWidth();
}
int Renderer::getViewportHeight() {
	return (int)d3d->getDefaultViewportHeight();
}
void Renderer::setClearColour(float r, float g, float b) {
	clear_colour.r = r;
	clear_colour.g = g;
	clear_colour.b = b;
}
void Renderer::setActiveCamera(Camera* cam) {
	active_camera = cam;
}
Camera* Renderer::getActiveCamera() {
	return active_camera;
}
void Renderer::setRenderTarget(RenderTexture* rt) {

	ID3D11RenderTargetView* target_view = rt->getView();

	getContext()->OMSetRenderTargets(1, &target_view, rt->getDepthStencilView());
	getContext()->RSSetViewports(1, &rt->getViewport());
}
void Renderer::enableDefaultRenderTarget() {
	d3d->defaultRenderTarget();
}
void Renderer::enableDefaultViewport() {
	d3d->defaultViewport();
}
void Renderer::shutdown() {
	if (d3d) {
		d3d->shutdown();
		delete d3d;
		d3d = 0;
	}
}