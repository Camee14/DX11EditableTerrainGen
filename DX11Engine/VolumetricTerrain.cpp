#include "VolumetricTerrain.h"
#include "LookupTables.h"

#include <algorithm>
#include <iterator>

/* tutorial from: http://paulbourke.net/geometry/polygonise/ */
VolumetricTerrain::VolumetricTerrain(int rad, double iso) : m_radius(rad), m_isolevel(iso) {
	m_diameter = rad * 2;
	m_numchunks = m_diameter * m_diameter * m_diameter;

	m_is_running = true;
}
VolumetricTerrain::~VolumetricTerrain() {
	delete[] m_data;
	//delete[] m_cache;
}
bool VolumetricTerrain::init(Renderer* render, float x, float z, int y_offset) {
	ShaderBuilder sb;
	sb.createVertexShader(L"res/shaders/VolTerrain.vs", "VTVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);
	sb.createPixelShader(L"res/shaders/VolTerrain.ps", "VTPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);

	sb.addElementToLayout("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	//sb.addElementToLayout("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	sb.addElementToLayout("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);

	sb.createBuffer("matrix_buffer", 0, VERTEX_SHADER);
	sb.setBufferDesc("matrix_buffer", D3D11_USAGE_DYNAMIC, sizeof(MatrixBuffer), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0);

	sb.createBuffer("light_buffer", 0, PIXEL_SHADER);
	sb.setBufferDesc("light_buffer", D3D11_USAGE_DYNAMIC, sizeof(LightBuffer), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0);

	sb.createBuffer("mat_buffer", 1, PIXEL_SHADER);
	sb.setBufferDesc("mat_buffer", D3D11_USAGE_DYNAMIC, sizeof(Material), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0);

	m_shader = new Shader();
	if (!sb.build(render, m_shader)) {
		return false;
	}
	m_offsetY = y_offset;

	setPos(x - (m_radius * CHUNK_SIZE), z - (m_radius * CHUNK_SIZE));

	m_material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // ambient
	m_material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // diffuse
	m_material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // specular
	m_material.SpecularPower = 45.f;

	m_center.x = x;
	m_center.y = z;

	m_data = new Chunk[m_numchunks];
	//m_cache = new Chunk[m_numchunks];

	m_worker_thd = std::thread(&VolumetricTerrain::procChunks, this, render->getDevice(), std::ref(m_is_running));

	return true;
}
void VolumetricTerrain::update(Renderer* render, float x, float z) {
	XMINT2 rel_pos;
	rel_pos.x = (x / CHUNK_SCALE) - m_center.x;
	rel_pos.y = (z / CHUNK_SCALE) - m_center.y;
	if ((rel_pos.x < -CHUNK_SIZE  || rel_pos.x > CHUNK_SIZE) || (rel_pos.y < -CHUNK_SIZE || rel_pos.y > CHUNK_SIZE)) {
		move(rel_pos);
	}
}
bool VolumetricTerrain::render(Renderer* render) {
	bool res;

	XMMATRIX viewProjectionMatrix = render->getViewMatrix() * render->getProjectionMatrix();

	int num_light = 0;
	LightBuffer lb = render->getLights(num_light)[0];
	lb.cameraPos.x = render->getActiveCamera()->transform.getPosition().x;
	lb.cameraPos.y = render->getActiveCamera()->transform.getPosition().y;
	lb.cameraPos.z = render->getActiveCamera()->transform.getPosition().z;
	lb.cameraPos.w = 1.f;

	m_shader->setActive(render->getContext());
	
	res = m_shader->setParams<LightBuffer>(render->getContext(), "light_buffer", lb);
	if (!res) {
		return false;
	}
	res = m_shader->setParams<Material>(render->getContext(), "mat_buffer", m_material);
	if (!res) {
		return false;
	}

	for (int i = 0; i < m_numchunks; i++) {
		Chunk* c = &m_data[i];

		if (!c->canRender()) {
			continue;
		}

		c->setActive(render->getContext());

		MatrixBuffer mb;
		mb.world = c->transform.getTransformMatrix();
		mb.WVP = mb.world * viewProjectionMatrix;
		mb.normal = XMMatrixTranspose(XMMatrixInverse(nullptr, mb.world));
		res = m_shader->setParams<MatrixBuffer>(render->getContext(), "matrix_buffer", mb);
		if (!res) {
			return false;
		}
		render->getContext()->Draw(c->getVertexCount(), 0);
	}
	return true;
}
void VolumetricTerrain::shutdown() {
	m_is_running = false;
	m_worker_thd.join();
	for (int i = 0; i < m_numchunks; i++) {
		m_data[i].shutdown();
	}

	if (m_shader) {
		m_shader->shutdown();
		m_shader = 0;
	}
}
void VolumetricTerrain::procChunks(ID3D11Device* device, std::atomic<bool>& is_running) {
	printf("start thread...\n");
	int rad_squared = m_radius * m_radius;

	while (is_running) {
		if (m_dir.x != 0 || m_dir.y != 0) {
			m_center.x += m_dir.x;
			m_center.y += m_dir.y;

			m_dir.x = 0;
			m_dir.y = 0;

			setPos(m_center.x - (m_radius * CHUNK_SIZE), m_center.y - (m_radius * CHUNK_SIZE));

			Chunk* next = new Chunk[m_numchunks];
			for (int i = 0; i < m_numchunks; i++) {
				int x = m_data[i].getIntegerPos().x - m_pos.x;
				int y = m_data[i].getIntegerPos().y;
				int z = m_data[i].getIntegerPos().z - m_pos.y;

				//if the chunk is still in the load area...
				if ((x >= 0 && x < (m_diameter * CHUNK_SIZE)) && (z >= 0 && z < (m_diameter * CHUNK_SIZE))) {
					//keep the chunk
					int index = getIndex(x / CHUNK_SIZE, y / CHUNK_SIZE, z / CHUNK_SIZE);
					if (index >= 0 && index < m_numchunks) {
						memcpy(&next[index], &m_data[i], sizeof(Chunk));
					}
				}
				else {
					m_destroy_queue.push(m_data[i]);
				}
			}
			std::swap(m_data, next);
			delete[] next;
			next = 0;
		}

		for (int i = 0; i < m_diameter; i++) {
			for (int j = 0; j < m_diameter; j++) {
				for (int k = 0; k < m_diameter; k++) {
					int index = getIndex(i, j, k);
					XMFLOAT2 lpos;
					lpos.x = i - m_radius;
					lpos.y = k - m_radius;

					if ((lpos.x * lpos.x) + (lpos.y * lpos.y) < rad_squared) 
					{
						if (!m_data[index].isInitalised())
						{
							m_data[index].init(i + (m_pos.x / CHUNK_SIZE), j, k + (m_pos.y / CHUNK_SIZE), m_diameter, m_offsetY);
							m_construct_queue.push(index);
						}
					}
				}
			}
		}
		if (m_construct_queue.size() > 0) {
			int index = m_construct_queue.front();
			m_construct_queue.pop();

			constructSurface(device, m_data[index]);
		}

		//std::memcpy(m_cache, m_data, sizeof(Chunk) * m_numchunks);

		while (m_cleanup_queue.size() > 0) {
			ID3D11Buffer* buffer = m_cleanup_queue.front();
			m_cleanup_queue.pop();

			buffer->Release();
			buffer = 0;
		}
		while(m_destroy_queue.size() > 0) {
			Chunk c = m_destroy_queue.front();
			m_destroy_queue.pop();

			c.shutdown();
		}
	}
	printf("end thread \n");
}
void VolumetricTerrain::move(XMINT2 dir) {
	if (dir.x < 0) {
		m_dir.x = -CHUNK_SIZE;
	}
	if (dir.x > 0) {
		m_dir.x = CHUNK_SIZE;
	}
	if (dir.y < 0) {
		m_dir.y = -CHUNK_SIZE;
	}
	if (dir.y > 0) {
		m_dir.y = CHUNK_SIZE;
	}
}
bool VolumetricTerrain::transformWorldToVoxel(float x, float y, float z, int& index, DirectX::XMINT3& indices) {
	return transformLoadAreaToVoxel(x / CHUNK_SCALE, y / CHUNK_SCALE, z / CHUNK_SCALE, index, indices);
}
bool VolumetricTerrain::transformLoadAreaToVoxel(float x, float y, float z, int& index, DirectX::XMINT3& indices) {
	x = x - m_pos.x;
	z = z - m_pos.y;

	index = getIndex((int)x / CHUNK_SIZE, (int)y / CHUNK_SIZE, (int)z / CHUNK_SIZE);
	if (index > 0 && index < m_numchunks) {
		XMINT3 cpos = m_data[index].getIntegerPos();
		indices.x = x - (cpos.x - m_pos.x);
		indices.y = y - cpos.y;
		indices.z = z - (cpos.z - m_pos.y);
		return true;
	}
	return false;
}
VOXEL VolumetricTerrain::getVoxelData(float wx, float wy, float wz) {
	int index;
	XMINT3 indices;
	if (transformLoadAreaToVoxel(wx / CHUNK_SCALE, wy / CHUNK_SCALE, wz / CHUNK_SCALE, index, indices)) {
		return getVoxelData(index, indices.x, indices.y, indices.z);
	}
	VOXEL v;
	v.density = 0;

	return v;
}
void VolumetricTerrain::setVoxelDensity(float, float, float, double) {

}
VOXEL VolumetricTerrain::getVoxelData(int i, int x, int y, int z) {
	return m_data[i].getVoxel(x, y, z);
}
void VolumetricTerrain::setVoxelData(int i, int x, int y, int z, double d) {
	m_data[i].setVoxel(x, y, z, d);
	m_construct_queue.push(i);
}
std::vector<VOXEL> VolumetricTerrain::getVoxelsInRadius(float wx, float wy, float wz, int rad) {
	std::vector<VOXEL> voxels;
	int d = rad * 2;
	int lx = (wx / CHUNK_SCALE) - rad;
	int ly = (wy / CHUNK_SCALE) - rad;
	int lz = (wz / CHUNK_SCALE) - rad;
	for (int x = lx; x < lx + d; x++) {
		for (int y = ly; y < ly + d; y++) {
			for (int z = lz; z < lz + d; z++) {
				int rx = x - (wx / CHUNK_SCALE);
				int ry = y - (wy / CHUNK_SCALE);
				int rz = z - (wz / CHUNK_SCALE);
				if (rx * rx + ry * ry + rz * rz < rad * rad) {
					int index = 0;
					XMINT3 indices;
					if (transformLoadAreaToVoxel(x, y, z, index, indices)) {
						//voxels.push_back(&getVoxelData(index, indices.x, indices.y, indices.z));
						double d = getVoxelData(index, indices.x, indices.y, indices.z).density;
						setVoxelData(index, indices.x, indices.y, indices.z, d - 0.05);
					}
				}
			}
		}
	}
	return voxels;
}
bool VolumetricTerrain::constructSurface(ID3D11Device* device, Chunk& current) {
	//for an individual chunk ... construct surface
	CUBE search;
	search.positions[0] = DirectX::XMINT3(-1, -1, 0);
	search.positions[1] = DirectX::XMINT3(0, -1, 0);
	search.positions[2] = DirectX::XMINT3(0, -1, -1);
	search.positions[3] = DirectX::XMINT3(-1, -1, -1);

	search.positions[4] = DirectX::XMINT3(-1, 0, 0);
	search.positions[5] = DirectX::XMINT3(0, 0, 0);
	search.positions[6] = DirectX::XMINT3(0, 0, -1);
	search.positions[7] = DirectX::XMINT3(-1, 0, -1);

	search.voxels[0] = current.getVoxel(-1, -1, 0);
	search.voxels[1] = current.getVoxel(0, -1, 0);
	search.voxels[2] = current.getVoxel(0, -1, -1);
	search.voxels[3] = current.getVoxel(-1, -1, -1);

	search.voxels[4] = current.getVoxel(-1, 0, 0);
	search.voxels[5] = current.getVoxel(0, 0, 0);
	search.voxels[6] = current.getVoxel(0, 0, -1);
	search.voxels[7] = current.getVoxel(-1, 0, -1);

	XMFLOAT3 pos = current.transform.getPosition();

	for (int i = 0; i < 8; i++) {
		XMFLOAT3 vox_world;
		vox_world.x = pos.x + search.positions[i].x;
		vox_world.y = pos.y + search.positions[i].y;
		vox_world.z = pos.z + search.positions[i].z;

		int index = 0;
		XMINT3 indices;
		float x(vox_world.x), y(vox_world.y), z(vox_world.z);
		if (transformWorldToVoxel(x, y, z, index, indices)) {
			search.voxels[i] = m_data[index].getVoxel(indices.x, indices.y, indices.z);
		}
		else {
			search.voxels[i] = current.getVoxel(search.positions[i].x, search.positions[i].y, search.positions[i].z);
		}
	}

	std::vector<TRIANGLE>chunk_ts;
	//printf("------------------------------- \n");
	do {
		std::vector<TRIANGLE> ts = calcutateStep(search);
		chunk_ts.insert(chunk_ts.end(), ts.begin(), ts.end()); //this is probably really slow...should fix for faster calculation of chunk surfaces
	} while (nextStep(search, current));

	if (chunk_ts.size() > 0) {
		ID3D11Buffer* pbuffer = 0;
		bool res = current.upload(device, chunk_ts.size(), &chunk_ts[0], pbuffer);
		if (!res) {
			return false;
		}
		if (pbuffer) {
			m_cleanup_queue.push(pbuffer);
		}
	}

	return true;
	//printf("------------------------------- \n");
}

std::vector<TRIANGLE> VolumetricTerrain::calcutateStep(CUBE search) {
	int index = getCubeIndex(search);
	std::vector<TRIANGLE> triangles;

	if (EdgeTable[index] == 0) {
		return triangles;
	}

	XMFLOAT3 vertices[12];
	if (EdgeTable[index] & 1) {
		vertices[0] = vecLerp(search.positions[0], search.positions[1], search.voxels[0], search.voxels[1]);
	}
	if (EdgeTable[index] & 2) {
		vertices[1] = vecLerp(search.positions[1], search.positions[2], search.voxels[1], search.voxels[2]);
	}
	if (EdgeTable[index] & 4) {
		vertices[2] = vecLerp(search.positions[2], search.positions[3], search.voxels[2], search.voxels[3]);
	}
	if (EdgeTable[index] & 8) {
		vertices[3] = vecLerp(search.positions[3], search.positions[0], search.voxels[3], search.voxels[0]);
	}
	if (EdgeTable[index] & 16) {
		vertices[4] = vecLerp(search.positions[4], search.positions[5], search.voxels[4], search.voxels[5]);
	}
	if (EdgeTable[index] & 32) {
		vertices[5] = vecLerp(search.positions[5], search.positions[6], search.voxels[5], search.voxels[6]);
	}
	if (EdgeTable[index] & 64) {
		vertices[6] = vecLerp(search.positions[6], search.positions[7], search.voxels[6], search.voxels[7]);
	}
	if (EdgeTable[index] & 128) {
		vertices[7] = vecLerp(search.positions[7], search.positions[4], search.voxels[7], search.voxels[4]);
	}
	if (EdgeTable[index] & 256) {
		vertices[8] = vecLerp(search.positions[0], search.positions[4], search.voxels[0], search.voxels[4]);
	}
	if (EdgeTable[index] & 512) {
		vertices[9] = vecLerp(search.positions[1], search.positions[5], search.voxels[1], search.voxels[5]);
	}
	if (EdgeTable[index] & 1024) {
		vertices[10] = vecLerp(search.positions[2], search.positions[6], search.voxels[2], search.voxels[6]);
	}
	if (EdgeTable[index] & 2048) {
		vertices[11] = vecLerp(search.positions[3], search.positions[7], search.voxels[3], search.voxels[7]);
	}

	for (int i = 0; TriangleTable[index][i] != -1; i += 3) {
		TRIANGLE t;
		t.points[0] = vertices[TriangleTable[index][i]];
		t.points[1] = vertices[TriangleTable[index][i + 1]];
		t.points[2] = vertices[TriangleTable[index][i + 2]];
		triangles.push_back(t);
	}
	return triangles;
}
int VolumetricTerrain::getCubeIndex(CUBE c) {
	int cubeindex = 0;
	if (c.voxels[0].density < m_isolevel) cubeindex |= 1;
	if (c.voxels[1].density < m_isolevel) cubeindex |= 2;
	if (c.voxels[2].density < m_isolevel) cubeindex |= 4;
	if (c.voxels[3].density < m_isolevel) cubeindex |= 8;
	if (c.voxels[4].density < m_isolevel) cubeindex |= 16;
	if (c.voxels[5].density < m_isolevel) cubeindex |= 32;
	if (c.voxels[6].density < m_isolevel) cubeindex |= 64;
	if (c.voxels[7].density < m_isolevel) cubeindex |= 128;

	return cubeindex;
}
bool VolumetricTerrain::nextStep(CUBE& search, Chunk& current) {
	if (search.positions[0].x == CHUNK_SIZE && search.positions[0].y == CHUNK_SIZE && search.positions[0].z == CHUNK_SIZE) {
		return false;
	} else if (search.positions[0].x == CHUNK_SIZE && search.positions[0].z == CHUNK_SIZE) {
		search.positions[0] = DirectX::XMINT3(-1, search.positions[0].y + 1, 0);
		search.positions[1] = DirectX::XMINT3(0, search.positions[1].y + 1, 0);
		search.positions[2] = DirectX::XMINT3(0, search.positions[2].y + 1, -1);
		search.positions[3] = DirectX::XMINT3(-1, search.positions[3].y + 1, -1);

		search.positions[4] = DirectX::XMINT3(-1, search.positions[4].y + 1, 0);
		search.positions[5] = DirectX::XMINT3(0, search.positions[5].y + 1, 0);
		search.positions[6] = DirectX::XMINT3(0, search.positions[6].y + 1, -1);
		search.positions[7] = DirectX::XMINT3(-1, search.positions[7].y + 1, -1);
	} else if (search.positions[0].x == CHUNK_SIZE) {
		search.positions[0] = DirectX::XMINT3(-1, search.positions[0].y, search.positions[0].z + 1);
		search.positions[1] = DirectX::XMINT3(0, search.positions[1].y, search.positions[1].z + 1);
		search.positions[2] = DirectX::XMINT3(0, search.positions[2].y, search.positions[2].z + 1);
		search.positions[3] = DirectX::XMINT3(-1, search.positions[3].y, search.positions[3].z + 1);

		search.positions[4] = DirectX::XMINT3(-1, search.positions[4].y, search.positions[4].z + 1);
		search.positions[5] = DirectX::XMINT3(0, search.positions[5].y, search.positions[5].z + 1);
		search.positions[6] = DirectX::XMINT3(0, search.positions[6].y, search.positions[6].z + 1);
		search.positions[7] = DirectX::XMINT3(-1, search.positions[7].y, search.positions[7].z + 1);
	}
	else {
		search.positions[0].x++;
		search.positions[1].x++;
		search.positions[2].x++;
		search.positions[3].x++;

		search.positions[4].x++;
		search.positions[5].x++;
		search.positions[6].x++;
		search.positions[7].x++;
	}
	
	XMINT3 pos = current.getIntegerPos();

	for (int i = 0; i < 8; i++) {
		XMINT3 vox_world;
		vox_world.x = pos.x + search.positions[i].x;
		vox_world.y = pos.y + search.positions[i].y;
		vox_world.z = pos.z + search.positions[i].z;

		int index = 0;
		XMINT3 indices;
		if(transformLoadAreaToVoxel(vox_world.x, vox_world.y, vox_world.z, index, indices)){
			search.voxels[i] = m_data[index].getVoxel(indices.x, indices.y, indices.z);
		}
		else {
			search.voxels[i] = current.getVoxel(search.positions[i].x, search.positions[i].y, search.positions[i].z);
		}
	}

	return true;
}
XMFLOAT3 VolumetricTerrain::vecLerp(DirectX::XMINT3 p1, DirectX::XMINT3 p2, VOXEL v1, VOXEL v2) {

	double mu;
	XMFLOAT3 p;

	if (std::abs(m_isolevel - v1.density) < 0.00001) {
		p.x = p1.x;
		p.y = p1.y;
		p.z = p1.z;
		return(p);
	}
	if (std::abs(m_isolevel - v2.density) < 0.00001) {
		p.x = p2.x;
		p.y = p2.y;
		p.z = p2.z;
		return(p);
	}
	if (std::abs(v1.density - v2.density) < 0.00001) {
		p.x = p1.x;
		p.y = p1.y;
		p.z = p1.z;
		return(p);
	}

	mu = (m_isolevel - v1.density) / (v2.density - v1.density);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);

	return p;
}


