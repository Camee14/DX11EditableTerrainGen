#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

#include "Direct3D.h"
#include "Camera.h"
#include "RenderTexture.h"
#include "ModelLoader.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

struct COLOUR {
	float r;
	float g;
	float b;
	float a;
};
struct MatrixBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX normal;
	DirectX::XMMATRIX WVP;
};
struct LightBuffer {
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 lightDirection;
	DirectX::XMFLOAT4 cameraPos;
};
class Renderer {
	public:
		Renderer();
		Renderer(const Renderer&);
		~Renderer();

		bool init(int, int, HWND);
		void shutdown();
		void begin();
		void end();

		void addLight(LightBuffer);
		LightBuffer* getLights(int&);

		void setClearColour(float, float, float);
		COLOUR getClearColor() const {
			return clear_colour;
		}
		void setActiveCamera(Camera*);
		void setRenderTarget(RenderTexture*);
		void enableDefaultRenderTarget();
		void enableDefaultViewport();
		void setAlphaBlending(bool);
		void setUseZBuffer(bool);

		int getViewportWidth();
		int getViewportHeight();
		Camera* getActiveCamera();

		ID3D11Device* getDevice();
		ID3D11DeviceContext* getContext();

		XMMATRIX getWorldMatrix();
		XMMATRIX getViewMatrix();
		XMMATRIX getProjectionMatrix();
		XMMATRIX getOrthoProjectionMatrix();

	private:
		Direct3D* d3d;
		COLOUR clear_colour;
		Camera* active_camera;
		std::vector<LightBuffer>m_lights;
};

#endif
