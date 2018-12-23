#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "Renderer.h"
#include "Rect.h"

#include "ShaderBuilder.h"

class PostProcessor {
public:
	PostProcessor();
	~PostProcessor();

	bool init(Renderer*, Shader*, Shader*);
	bool process(Renderer*);
	void shutdown();

	void setActiveCamera(Camera*);

	RenderTexture* getRenderTexture() const {
		return m_render_tex;
	}
	void clearRenderTexture(Renderer*);

private:
	struct MatrixBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	bool downScale(Renderer*);
	bool upScale(Renderer*);
	bool renderFinal(Renderer*);

	double m_scale;
	RenderTexture *m_render_tex, *m_downscale_tex, *m_upscale_tex, *m_final_tex;

	Rect* m_small_window;
	Rect* m_large_window;

	Camera* m_cam;

	Shader* m_proc_shader;
	Shader* m_basic_shader;
};

#endif
