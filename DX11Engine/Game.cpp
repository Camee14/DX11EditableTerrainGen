#include "Game.h"

#include "Noise.h"

bool Game::gameStart() {
	bool res;

	main_camera = new Camera;
	if (!main_camera) {
		return false;
	}
	main_camera->transform.setPosition(0.0f, 250.0f, 0.0f);
	renderer->setActiveCamera(main_camera);

	velY = 0;
	is_grounded = false;

	/*ShaderBuilder sb;

	sb.createVertexShader(L"res/shaders/text.vs", "textVertex", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);
	sb.createPixelShader(L"res/shaders/text.ps", "textPixel", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0);

	sb.addElementToLayout("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
	sb.addElementToLayout("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	sb.addElementToLayout("SCREEN", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1);
	
	sb.createBuffer("ft_buffer", 0, VERTEX_SHADER);
	sb.setBufferDesc("ft_buffer", D3D11_USAGE_DYNAMIC, sizeof(FONTBUFFER), D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0);

	sb.createSampler("ss");
	sb.setSamplerDesc("ss", D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP, 0.0f, 1, D3D11_COMPARISON_ALWAYS, XMFLOAT4(0.f, 0.f, 0.f, 0.f), 0, D3D11_FLOAT32_MAX);

	font_shader = new Shader();
	if (!sb.build(renderer, font_shader)) {
		return false;
	}*/

	vt = new VolumetricTerrain(14, 0.3);
	if (!vt) {
		return false;
	}
	res = vt->init(renderer, main_camera->transform.getPosition().x, main_camera->transform.getPosition().z, 0);
	if (!res) {
		return false;
	}

	/*font = new Font();
	res = loadFontFromFile(font, "res/fonts/TrebuchetMS_32.fnt");
	if (!res) {
		return false;
	}
	res = font->init(renderer->getDevice(), renderer->getContext());
	if (!res) {
		return false;
	}

	text = new Text(font, font_shader, 0, 0);
	text->init();
	res = text->setText(renderer->getDevice(), "hello world");
	if (!res) {
		return false;
	}*/

	post_processor = new PostProcessor();
	res = post_processor->init(renderer, nullptr, nullptr);
	if (!res) {
		return false;
	}
	post_processor->setActiveCamera(main_camera);

	LightBuffer lb;
	lb.ambientColor = XMFLOAT4(.5f, .5f, .5f, 1.f);
	lb.diffuseColor = XMFLOAT4(1.f, 1.f, 1.f, 1.0f);
	lb.lightDirection = XMFLOAT4(0.0f, -1.0f, 3.0f, 0.0f);
	renderer->addLight(lb);

	return true;
}
bool Game::gameUpdate(float dt) {
	int mouse_pos_x, mouse_pos_y;
	im->onMouseMove(mouse_pos_x, mouse_pos_y);

	float angle_h = 5.0f * float(mouse_pos_x) * dt;
	float angle_v = 5.0f * float(mouse_pos_y) * dt;

	main_camera->transform.rotatePYR(angle_v, angle_h, 0.0f);

	XMFLOAT3 cam_rot = main_camera->transform.getRotation();
	XMMATRIX rot_matrix;
	//rot_matrix = DirectX::XMMatrixRotationRollPitchYaw(cam_rot.x * 0.0174532925f, cam_rot.y * 0.0174532925f, cam_rot.z * 0.0174532925f);
	rot_matrix = DirectX::XMMatrixRotationRollPitchYaw(0, cam_rot.y * 0.0174532925f, 0);

	XMFLOAT3 cam_translate;
	XMVECTOR cam_translate_vec;

	cam_translate.x = 0.0f;
	cam_translate.y = 0.0f;
	cam_translate.z = 0.0f;

	if (im->onKeyPressed(DIK_D)) {//right
		cam_translate.x = 50.0f * dt;
	}
	if (im->onKeyPressed(DIK_A)) {//left
		cam_translate.x = -50.0f * dt;
	}
	if (im->onKeyPressed(DIK_W)) {//forward
		cam_translate.z = 50.0f * dt;
	}
	if (im->onKeyPressed(DIK_S)) {//back
		cam_translate.z = -50.0f * dt;
	}

	if (!is_grounded) {
		velY -= (8 * 9.81) * dt;
		cam_translate.y = velY * dt;
	}

	cam_translate_vec = XMLoadFloat3(&cam_translate);
	cam_translate_vec = XMVector3TransformCoord(cam_translate_vec, rot_matrix);

	main_camera->transform.translate(XMVectorGetX(cam_translate_vec), XMVectorGetY(cam_translate_vec), XMVectorGetZ(cam_translate_vec));
	
	is_grounded = false;
	XMFLOAT3 foot_pos = main_camera->transform.getPosition();
	foot_pos.y -= 5;
	int index = 0;
	XMINT3 indices;
	if (vt->transformWorldToVoxel(
		foot_pos.x, 
		foot_pos.y,
		foot_pos.z,
		index, 
		indices)
	)
	{
		XMFLOAT3 pos1 = foot_pos;
		double d1 = vt->getVoxelData(pos1.x, pos1.y, pos1.z).density;
		double d2 = 0;
		if (d1 >= 0.3) {
			velY = 0;
			bool found = false;
			XMFLOAT3 pos2 = pos1;
			while (!found) {
				pos2.y++;
				d2 = vt->getVoxelData(pos2.x, pos2.y, pos2.z).density;
				if (d2 <= 0.3) {
					found = true;
				}
			}
			is_grounded = true;
			main_camera->transform.setPosition(pos2.x, std::floor(pos2.y) + 5, pos2.z);
		}
	}
	else {
		printf("fallen through map\n");
	}

	rot_matrix = DirectX::XMMatrixRotationRollPitchYaw(cam_rot.x * 0.0174532925f, cam_rot.y * 0.0174532925f, cam_rot.z * 0.0174532925f);

	XMFLOAT3 aim;
	aim.x = 0;
	aim.y = 0;
	aim.z = 20;

	XMVECTOR aim_vec = XMVector3TransformCoord(XMLoadFloat3(&aim), rot_matrix);
	XMStoreFloat3(&aim, aim_vec);

	aim.x += main_camera->transform.getPosition().x;
	aim.y += main_camera->transform.getPosition().y;
	aim.z += main_camera->transform.getPosition().z;

	if (im->onKeyDown(DIK_F)) 
	{
		vt->getVoxelsInRadius(aim.x, aim.y, aim.z, 3);
	}
	if(im->onKeyPressed(DIK_SPACE)){
		index = 0;
		indices.x = 0;
		indices.y = 0;
		indices.z = 0;
		if (vt->transformWorldToVoxel(aim.x, aim.y, aim.z, index, indices)) 
		{
			vt->setVoxelData(index, indices.x, indices.y, indices.z, 1.0);
		}
	}
	main_camera->update();

	vt->update(renderer, main_camera->transform.getPosition().x, main_camera->transform.getPosition().z);

	return true;
}
bool Game::gameDraw() {
	bool res = false;

	renderer->begin();

	post_processor->clearRenderTexture(renderer);
	renderer->setRenderTarget(post_processor->getRenderTexture());

	//render the terrain
	res = vt->render(renderer);
	if (!res) {
		return false;
	}

	renderer->enableDefaultRenderTarget();
	renderer->enableDefaultViewport();

	post_processor->process(renderer);

	//renderer->setAlphaBlending(true);
	//text->render(renderer->getContext(), renderer->getOrthoProjectionMatrix());
	//renderer->setAlphaBlending(false);

	renderer->end();

	return true;
}
void Game::gameEnd() {
	if (vt) {
		vt->shutdown();
		delete vt;
		vt = 0;
	}
	/*if (font) {
		font->shutdown();
		delete font;
		font = 0;
	}
	if (text) {
		text->shutdown();
		delete text;
		text = 0;
	}*/
	if (main_camera) {
		delete main_camera;
		main_camera = 0;
	}
}