#ifndef TEXTURE_H
#define TEXTURE_H

#include <d3d11.h>
#include <stdio.h>

class Texture {
public:
	Texture();
	Texture(const Texture&);
	~Texture();

	bool init(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void shutdown();

	ID3D11ShaderResourceView* getShaderResource();

private:
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_texture_view;
};

#endif
