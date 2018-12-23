#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <d3d11.h>
#include <directxmath.h>

#include <string>
#include <map>

#include "Texture.h"
#include "Shader.h"
struct FONTBUFFER{
	DirectX::XMMATRIX projection;
};
class Font{
public:
	Font();
	~Font();

	bool init(ID3D11Device*, ID3D11DeviceContext*);
	void shutdown();

	bool getKerningValue(char, char, int&);
	bool getOffsetValues(char, int&, int&, int&);

	ID3D11Buffer* getCharacterBuffer(char);
	ID3D11ShaderResourceView* getTextureResource();

	int getVBOSize();
	int getVBOCountPerInstance();

	void print();
private:
	struct VBO{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
	};
	struct CHARACTER {
		CHARACTER() {
			bounds = DirectX::XMFLOAT4(0, 0, 0, 0);
			offset = DirectX::XMFLOAT2(0, 0);
			int advance = 0;
		}
		DirectX::XMFLOAT4 bounds;
		DirectX::XMFLOAT2 offset;
		int advance;
	};

	//friend bool setValue(Font*, unsigned char type, std::string, std::string);
	friend bool setValues(Font*, std::map<std::string, std::string>, std::vector< std::map<std::string, std::string> >, std::vector< std::map<std::string, std::string> >);
	Texture* m_tex;

	std::map<char, CHARACTER>m_characters;
	std::map<std::pair<char, char>, float>m_kernings;

	std::string m_face, m_charset, m_tex_path;
	int m_size, m_stretchH, m_smooth, m_aa, m_lineH, m_base, m_scaleH, m_scaleW;
	DirectX::XMINT4 m_padding;
	DirectX::XMINT2 m_spacing;
	bool m_unicode, m_bold, m_italic;

	std::map<char, ID3D11Buffer*> m_vertex_buffers;
	int m_vertex_count;
};

class Text {
public:
	Text(Font*, Shader*, float, float);
	~Text();

	bool init();
	void shutdown();

	void setPosition(float, float);
	bool setText(ID3D11Device*, std::string);
	void render(ID3D11DeviceContext*, DirectX::XMMATRIX);
private:
	struct IBO {
		DirectX::XMFLOAT3 position;
	};
	struct INSTANCE {
		INSTANCE() {
			buffer = 0;
			instances = 0;
			character = 0;
		}
		ID3D11Buffer* buffer;
		char character;
		int instances;
		std::vector<IBO> ibo;
	};
	bool addCharToInstanceList(char, INSTANCE&, int&, char = 0);
	void clear();

	Font* m_font_ptr;
	Shader* m_shader_ptr;
	DirectX::XMFLOAT2 m_pos;

	std::vector<INSTANCE> m_instances;
};
bool loadFontFromFile(Font*, std::string);
#endif;
