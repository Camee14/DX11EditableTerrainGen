#include "Particles.h"
//random double function from https://stackoverflow.com/questions/2704521/generate-random-double-numbers-in-c
double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

ParticleSystem::ParticleSystem(Shader* s, Emitter e) {
	m_shader = s;
	m_emitter = e;

	m_vertex_shader = 0;
	m_geo_shader = 0;
	m_stream_buffer = 0;
	m_draw_buffer = 0;
	m_emitter_buffer = 0;
	m_gen_buffer = 0;
}
ParticleSystem::~ParticleSystem() {

}
bool ParticleSystem::init(ID3D11Device* device, WCHAR* path_vertex, const char* entry_vertex, WCHAR* path_geo, const char* entry_geo) {
	HRESULT res;

	ID3D10Blob* m_vertex_buffer = 0;
	ID3D10Blob* m_geo_buffer = 0;

	res = D3DCompileFromFile(path_vertex, NULL, NULL, entry_vertex, "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &m_vertex_buffer, &m_error_msg);
	if (FAILED(res)) {
		if (m_error_msg) {
			outputShaderErrorMsg(m_error_msg, path_vertex);
		}
		else {
			printf("cound not find vertex shader file: %s \n", path_vertex);
		}
		return false;
	}

	res = D3DCompileFromFile(path_geo, NULL, NULL, entry_geo, "gs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &m_geo_buffer, &m_error_msg);
	if (FAILED(res)) {
		if (m_error_msg) {
			outputShaderErrorMsg(m_error_msg, path_geo);
		}
		else {
			printf("cound not find vertex shader file: %s \n", path_geo);
		}
		return false;
	}

	res = device->CreateVertexShader(m_vertex_buffer->GetBufferPointer(), m_vertex_buffer->GetBufferSize(), NULL, &m_vertex_shader);
	if (FAILED(res)) {
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "DIRECTION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },//direction
		{ "INFORMATION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },// info, x = age y = type
	};
	int numElements = sizeof(layout) / sizeof(layout[0]);
	res = device->CreateInputLayout(layout, numElements, m_vertex_buffer->GetBufferPointer(), m_vertex_buffer->GetBufferSize(), &m_layout);
	if (FAILED(res)) {
		return false;
	}

	m_vertex_buffer->Release();

	D3D11_SO_DECLARATION_ENTRY pDecl[] =
	{
		// semantic name, semantic index, start component, component count, output slot
		{ 0, "POSITION", 0, 0, 4, 0}, // output all components of position
		{ 0, "DIRECTION", 0, 0, 4, 0 },  // outout all components of direction  
		{ 0, "INFORMATION", 0, 0, 2, 0 },  //output all components of info
	};

	UINT stride = sizeof(PARTICLE);
	res = device->CreateGeometryShaderWithStreamOutput(
		m_geo_buffer->GetBufferPointer(),
		m_geo_buffer->GetBufferSize(),
		pDecl,
		ARRAYSIZE(pDecl),
		&stride,
		1,
		D3D11_SO_NO_RASTERIZED_STREAM,
		NULL,
		&m_geo_shader
	);
	if (FAILED(res)) {
		return false;
	}
	m_geo_buffer->Release();

	D3D11_BUFFER_DESC vertex_buffer_desc;
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(PARTICLE);
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	PARTICLE p[1];
	ZeroMemory(&p, sizeof(p));
	p[0].position = DirectX::XMFLOAT4(-3.5f, 0.f, 0.f, 1.0f);
	p[0].direction = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.0f);
	p[0].info.x = 0.f; //set age of particle
	p[0].info.y = 0.f; // set the type of the particle

	D3D11_SUBRESOURCE_DATA init_particles;
	init_particles.pSysMem = p;
	init_particles.SysMemPitch = 0;
	init_particles.SysMemSlicePitch = 0;

	res = device->CreateBuffer(&vertex_buffer_desc, &init_particles, &m_init_buffer);
	if (FAILED(res)) {
		return false;
	}

	vertex_buffer_desc.ByteWidth = sizeof(PARTICLE) * MAX_NUM_PARTICLES;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	res = device->CreateBuffer(&vertex_buffer_desc, NULL, &m_draw_buffer);
	if (FAILED(res)) {
		return false;
	}
	res = device->CreateBuffer(&vertex_buffer_desc, NULL, &m_stream_buffer);
	if (FAILED(res)) {
		return false;
	}
	m_num_init_particles = 1;

	D3D11_BUFFER_DESC const_buffer_desc;
	const_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	const_buffer_desc.ByteWidth = sizeof(Emitter);
	const_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	const_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	const_buffer_desc.StructureByteStride = 0;
	const_buffer_desc.MiscFlags = 0;
	res = device->CreateBuffer(&const_buffer_desc, NULL, &m_emitter_buffer);
	if (FAILED(res)) {
		return false;
	}

	const_buffer_desc.ByteWidth = sizeof(GENERATOR);
	res = device->CreateBuffer(&const_buffer_desc, NULL, &m_gen_buffer);
	if (FAILED(res)) {
		return false;
	}

	return true;
}
void ParticleSystem::shutdown() {
	if (m_layout) {
		m_layout->Release();
		m_layout = 0;
	}
	if (m_vertex_shader) {
		m_vertex_shader->Release();
		m_vertex_shader = 0;
	}
	if (m_geo_shader) {
		m_geo_shader->Release();
		m_geo_shader = 0;
	}
	if (m_init_buffer) {
		m_init_buffer->Release();
		m_init_buffer = 0;
	}
	if (m_stream_buffer) {
		m_stream_buffer->Release();
		m_stream_buffer = 0;
	}
	if (m_draw_buffer) {
		m_draw_buffer->Release();
		m_draw_buffer = 0;
	}
	if (m_emitter_buffer) {
		m_emitter_buffer->Release();
		m_emitter_buffer = 0;
	}
	if (m_gen_buffer) {
		m_gen_buffer->Release();
		m_gen_buffer = 0;
	}
}
void ParticleSystem::update(float dt) {
	//code from https://gist.github.com/andrewbolster/10274979
	float phi = fRand(0, 3.14 * 2);
	float costheta = fRand(-1, 1);

	float theta = acos(costheta);
	float x = sin(theta) * cos(phi);
	float y = sin(theta) * sin(phi);
	float z = cos(theta);

	m_gen.next_dir = DirectX::XMFLOAT4(x, y, z, 0.f);
	m_gen.delta_time = dt;
}
void ParticleSystem::render(ID3D11DeviceContext* context, DirectX::XMMATRIX VP) {
	UINT stride = sizeof(PARTICLE);
	UINT offset = 0;
	//set the vertex buffer and input layout
	context->IASetInputLayout(m_layout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	setEmitter(context, m_emitter);
	setGenerator(context, m_gen);

	if (m_init_buffer) {
		context->IASetVertexBuffers(0, 1, &m_init_buffer, &stride, &offset);
	}
	else {
		context->IASetVertexBuffers(0, 1, &m_draw_buffer, &stride, &offset);
	}
	
	//set the stream out buffer and the gen shader
	context->SOSetTargets(1, &m_stream_buffer, &offset);
	context->GSSetShader(m_geo_shader, NULL, 0);
	context->VSSetShader(m_vertex_shader, NULL, 0);

	//generate the particles
	if (m_init_buffer) {
		context->Draw(m_num_init_particles, 0);
		m_init_buffer->Release();
		m_init_buffer = 0;
	}
	else {
		context->DrawAuto();
	}

	//swap the buffers
	std::swap(m_draw_buffer, m_stream_buffer);

	//unbind the stream output buffer
	ID3D11Buffer* pBuffer[1] = { 0 };
	context->SOSetTargets(1, pBuffer, &offset);

	//set the vertex buffer
	context->IASetVertexBuffers(0, 1, &m_draw_buffer, &stride, &offset);

	//set the draw buffer
	PSBUFFER psb;
	psb.VP = VP;
	m_shader->setParams<PSBUFFER>(context, "ps_buffer", psb);
	m_shader->setParams<Emitter>(context, "emitter", m_emitter);
	m_shader->setActive(context);

	//draw the particles
	context->DrawAuto();

	//unbind the geometry shader
	context->GSSetShader(nullptr, NULL, 0);
}
void ParticleSystem::addParticleEmitter(DirectX::XMFLOAT3 pos) {
	PARTICLE p;
	ZeroMemory(&p, sizeof(PARTICLE));
	//p.info.x = 0.f; //set age of particle
	//p.info.y = 0.f; // set the type of the particle

}
void ParticleSystem::setEmitter(ID3D11DeviceContext* context, Emitter e) {
	HRESULT res;
	D3D11_MAPPED_SUBRESOURCE mapped_resource;

	res = context->Map(m_emitter_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(res)) {
		return;
	}

	Emitter* ptr = (Emitter*)mapped_resource.pData;
	memcpy(ptr, &e, sizeof(e));

	context->Unmap(m_emitter_buffer, 0);

	context->GSSetConstantBuffers(0, 1, &m_emitter_buffer);
}
void ParticleSystem::setGenerator(ID3D11DeviceContext* context, GENERATOR g) {
	HRESULT res;
	D3D11_MAPPED_SUBRESOURCE mapped_resource;

	res = context->Map(m_gen_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(res)) {
		return;
	}

	GENERATOR* ptr = (GENERATOR*)mapped_resource.pData;
	memcpy(ptr, &g, sizeof(g));

	context->Unmap(m_gen_buffer, 0);

	context->GSSetConstantBuffers(1, 1, &m_gen_buffer);
}
void ParticleSystem::outputShaderErrorMsg(ID3D10Blob* msg, WCHAR* file) {
	char* compile_errors;
	unsigned long long buffer_size, i;
	std::ofstream fout;

	compile_errors = (char*)(msg->GetBufferPointer());

	buffer_size = msg->GetBufferSize();

	fout.open("shader_error.txt");

	for (i = 0; i < buffer_size; i++) {
		fout << compile_errors[i];
	}
	fout.close();

	msg->Release();
	msg = 0;

	printf("error compiling shader, error message exported to shader_error.txt \n");
}
