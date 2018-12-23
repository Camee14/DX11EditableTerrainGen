#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

#include "Shader.h"
#include "Texture.h"
#include "Transform.h"

#define MAX_NUM_PARTICLES 3000
struct PSBUFFER {
	DirectX::XMMATRIX VP;//64 bytes
};
struct Emitter {
	DirectX::XMFLOAT4 start_colour;
	DirectX::XMFLOAT4 end_colour;
	float start_velocity;
	float end_velocity;
	float mass;
	float lifespan;
	float emission_rate;

	DirectX::XMFLOAT3 padding;
};
class ParticleSystem {
public:
	ParticleSystem(Shader*, Emitter);
	~ParticleSystem();

	bool init(ID3D11Device*, WCHAR*, const char*, WCHAR*, const char*);
	void shutdown();

	void update(float);
	void render(ID3D11DeviceContext*, DirectX::XMMATRIX);
	
	void addParticleEmitter(DirectX::XMFLOAT3);

	Transform transform;
private:
	struct PARTICLE {
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 direction;
		DirectX::XMFLOAT2 info;//x = age y = type

		//DirectX::XMFLOAT2 padding;
	};
	struct GENERATOR {
		DirectX::XMFLOAT4 next_dir;
		float delta_time;

		DirectX::XMFLOAT3 padding;
	};
	void ParticleSystem::setEmitter(ID3D11DeviceContext*, Emitter);
	void ParticleSystem::setGenerator(ID3D11DeviceContext*, GENERATOR);

	void outputShaderErrorMsg(ID3D10Blob*, WCHAR*);

	Shader* m_shader;
	Emitter m_emitter;
	GENERATOR m_gen;

	ID3D10Blob* m_error_msg;

	ID3D11InputLayout* m_layout;

	ID3D11VertexShader* m_vertex_shader;
	ID3D11GeometryShader* m_geo_shader;

	//vertex buffers
	ID3D11Buffer* m_init_buffer;
	ID3D11Buffer* m_stream_buffer;
	ID3D11Buffer* m_draw_buffer;

	//constant buffers
	ID3D11Buffer* m_emitter_buffer;
	ID3D11Buffer* m_gen_buffer;

	UINT m_num_init_particles;
};

#endif
