#ifndef DIRECT3D_H
#define DIRECT3D_H

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

class Direct3D {
	public:
		Direct3D();
		Direct3D(const Direct3D&);
		~Direct3D();

		bool init(int, int, bool, HWND, bool, float, float);
		void shutdown();

		void beginScene(float, float, float, float);
		void endScene();

		void alphaBlending(bool);
		void zBuffer(bool);
		void defaultRenderTarget();
		void defaultViewport();

		ID3D11Device* getDevice();
		ID3D11DeviceContext* getDeviceContext();

		void getProjectionMatrix(XMMATRIX&);
		void getWorldMatrix(XMMATRIX&);
		void getOrthoMatrix(XMMATRIX&);

		float getDefaultViewportWidth();
		float getDefaultViewportHeight();

		void getVideoCardInfo(char*, int&);
	private:
		bool v_sync;
		int video_card_mem;
		char video_card_desc[128];

		IDXGISwapChain* swap_chain;
		ID3D11Device* device;
		ID3D11DeviceContext* device_context;
		ID3D11RenderTargetView* render_target_view;
		ID3D11Texture2D* depth_stencil_buffer;
		ID3D11DepthStencilState* depth_stencil_state_enabled;
		ID3D11DepthStencilState* depth_stencil_state_disabled;
		ID3D11DepthStencilView* depth_stencil_view;
		ID3D11RasterizerState* raster_state;

		ID3D11BlendState* m_alpha_blend_state_enable;
		ID3D11BlendState* m_alpha_blend_state_disable;

		XMMATRIX projection_matrix;
		XMMATRIX world_matrix;
		XMMATRIX ortho_matrix;

		D3D11_VIEWPORT viewport;
};
#endif
