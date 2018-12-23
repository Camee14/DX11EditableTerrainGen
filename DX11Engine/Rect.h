#ifndef RECT_H
#define RECT_H

#include "Renderer.h"

class Rect {
public:
	Rect();
	~Rect();

	bool init(ID3D11Device*, int, int);
	void render(Renderer*);

	void shutdown();
private:
	struct VBO {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	ID3D11Buffer *m_vertex_buffer;
	unsigned int m_vertex_count, m_stride;
};

#endif