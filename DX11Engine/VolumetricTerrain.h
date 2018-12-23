#ifndef VOLUMETRICTERRAIN_H
#define VOLUMETRICTERRAIN_H

#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

#include "Chunk.h"
#include "ShaderBuilder.h"
//#include "TerrainWorker.h"

class TerrainWorkerThread;

class VolumetricTerrain {
public:
	VolumetricTerrain(int, double);
	~VolumetricTerrain();

	bool init(Renderer*, float, float, int);
	void update(Renderer*, float x, float z);
	bool render(Renderer*);
	void shutdown();

	bool transformWorldToVoxel(float x, float y, float z, int& index, DirectX::XMINT3& indices);
	bool transformLoadAreaToVoxel(float x, float y, float z, int& index, DirectX::XMINT3& indices);

	std::vector<VOXEL> getVoxelsInRadius(float, float, float, int);

	VOXEL getVoxelData(float, float, float);
	void setVoxelDensity(float, float, float, double);

	VOXEL getVoxelData(int, int, int, int);
	void setVoxelData(int, int, int, int, double);

	bool constructSurface(ID3D11Device*, Chunk&);
private:
	struct CUBE{
		DirectX::XMINT3 positions[8];
		VOXEL voxels[8];
	};
	int getIndex(int x, int y, int z) {
		return x + (m_diameter * y) + ((m_diameter * m_diameter)*z);
	}
	void setPos(int x, int y) {
		m_pos.x = x;
		m_pos.y = y;
	}

	int getCubeIndex(CUBE);

	std::vector<TRIANGLE> calcutateStep(CUBE);
	DirectX::XMFLOAT3 vecLerp(DirectX::XMINT3, DirectX::XMINT3, VOXEL, VOXEL);
	bool nextStep(CUBE&, Chunk&);

	void move(XMINT2);
	bool neighboursAreInit(Chunk);

	int m_radius;
	int m_diameter;
	int m_numchunks;
	double m_isolevel;

	DirectX::XMINT2 m_center;
	DirectX::XMINT2 m_dir;
	DirectX::XMINT2 m_pos;
	int m_offsetY;

	Material m_material;

	Shader* m_shader;
	Chunk* m_data;
	Chunk* m_cache;

	void procChunks(ID3D11Device*, std::atomic<bool>&);
	std::thread m_worker_thd;
	std::mutex m_mutex;

	std::queue<int> m_construct_queue;
	std::queue<Chunk> m_destroy_queue;
	std::queue<ID3D11Buffer*>m_cleanup_queue;

	std::atomic<bool> m_is_running;
};

#endif
