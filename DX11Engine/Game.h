#ifndef GAME_H
#define GAME_H

#include "Engine.h"

#include "Camera.h"
#include "FontLoader.h"
#include "ModelLoader.h"
#include "Shader.h"
#include "ShaderBuilder.h"
#include "Texture.h"
#include "Particles.h"
#include "Terrain.h"
#include "VolumetricTerrain.h"
#include "PostProcessor.h"

class Game : public Engine {
public:
	bool gameStart();
	bool gameUpdate(float);
	bool gameDraw();
	void gameEnd();
private:
	//Shader* font_shader;
	//Shader* texture_shader;

	PostProcessor* post_processor;

	//Font* font;
	//Text* text;

	Camera* main_camera;
	float velY;
	bool is_grounded;

	VolumetricTerrain* vt;
};

#endif
