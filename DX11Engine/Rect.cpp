#include "Rect.h"

using namespace DirectX;

Rect::Rect() {
	m_vertex_buffer = 0;
	m_vertex_count = 0;
	m_stride = 0;
}
Rect::~Rect() {

}
bool Rect::init(ID3D11Device* device, int width, int height) {
	float left, right, top, bottom;

	right = width * 0.5f;
	left = right * -1.0f;

	top = height * 0.5f;
	bottom = top * -1.0f;

	m_vertex_count = 6;
	m_stride = sizeof(VBO);

	VBO* vbo = new VBO[m_vertex_count];

	vbo[0].pos = XMFLOAT3(left, top, SCREEN_NEAR);  // Top left.
	vbo[0].uv = XMFLOAT2(0.0f, 0.0f);

	vbo[1].pos = XMFLOAT3(right, bottom, SCREEN_NEAR);  // Bottom right.
	vbo[1].uv = XMFLOAT2(1.0f, 1.0f);

	vbo[2].pos = XMFLOAT3(left, bottom, SCREEN_NEAR);  // Bottom left.
	vbo[2].uv = XMFLOAT2(0.0f, 1.0f);

	vbo[3].pos = XMFLOAT3(left, top, SCREEN_NEAR);  // Top left.
	vbo[3].uv = XMFLOAT2(0.0f, 0.0f);

	vbo[4].pos = XMFLOAT3(right, top, SCREEN_NEAR);  // Top right.
	vbo[4].uv = XMFLOAT2(1.0f, 0.0f);

	vbo[5].pos = XMFLOAT3(right, bottom, SCREEN_NEAR);  // Bottom right.
	vbo[5].uv = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC vertex_buffer_desc;
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = m_stride * m_vertex_count;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_data;
	vertex_data.pSysMem = vbo;
	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	HRESULT res;
	res = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &m_vertex_buffer);
	if (FAILED(res)) {
		return false;
	}

	delete[] vbo;
	vbo = 0;

	return true;
}
void Rect::render(Renderer* renderer) {
	renderer->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	unsigned int offset = 0;
	renderer->getContext()->IASetVertexBuffers(0, 1, &m_vertex_buffer, &m_stride, &offset);

	renderer->getContext()->Draw(m_vertex_count, 0);
}
void Rect::shutdown() {
	if (m_vertex_buffer) {
		m_vertex_buffer->Release();
		m_vertex_buffer = 0;
	}
}