#ifndef CHUNK_H
#define CHUNK_H

#include <d3d11.h>
#include <directxmath.h>

#include "Renderer.h"
#include "Transform.h"

#define CHUNK_SIZE 8
#define CHUNK_SCALE 5

struct VOXEL {
	double density;
};
struct TRIANGLE {
	DirectX::XMFLOAT3 points[3];
};

class Chunk {
public:
	Chunk();
	~Chunk();

	void init(int, int, int, int, int);
	bool upload(ID3D11Device*, unsigned int, TRIANGLE*, ID3D11Buffer*);
	void setActive(ID3D11DeviceContext*);
	void shutdown();

	VOXEL getVoxel(int, int, int);
	void setVoxel(int, int, int, double);
	void refresh();

	void destroy() {
		m_destroy = true;
	}

	int getVertexCount() const {
		return m_vertex_count;
	}
	bool isConstructed() const {
		return m_can_render || m_is_air;
	}
	bool isInitalised() const {
		return m_is_init;
	}
	bool markedDestroy() const {
		return m_destroy;
	}
	bool canRender() const {
		return m_can_render;
	}
	bool canConstruct() const {
		return isInitalised() && !isConstructed() && !markedDestroy();
	}

	XMINT3 getIntegerPos() const {
		return m_int_pos;
	}
	void setIntegerPos(int x, int y, int z) {
		m_int_pos.x = x;
		m_int_pos.y = y; 
		m_int_pos.z = z;
	}
	void setIsAir(){
		m_is_air = true;
	}

	Transform transform;
private:
	struct VBO {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
	};
	/*
	Tutorial from: https://stackoverflow.com/questions/10903149/how-do-i-compute-the-linear-index-of-a-3d-coordinate-and-vice-versa
	*/
	int getIndex(int x, int y, int z) {
		return x + (CHUNK_SIZE * y) + ((CHUNK_SIZE * CHUNK_SIZE)*z);
	}

	VOXEL* m_data;

	bool m_is_air;
	bool m_can_render;
	bool m_is_init;
	bool m_destroy;

	int m_vertex_count;
	int m_stride;

	XMINT3 m_int_pos;

	ID3D11Buffer *m_vertex_buffer;
};

#endif
