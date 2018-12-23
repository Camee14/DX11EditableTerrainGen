#include "FontLoader.h"

#include "utils.h"

#include <istream>
#include <sstream>
#include <regex>
/*
tutorial from: https://www.braynzarsoft.net/viewtutorial/q16390-11-drawing-text-in-directx-12
*/
enum FILE_LINE_TYPES {
	FILE_LINE_TYPE_UNKNOWN,
	FILE_LINE_TYPE_INFO,
	FILE_LINE_TYPE_COMMON,
	FILE_LINE_TYPE_PAGE,
	FILE_LINE_TYPE_CHAR,
	FILE_LINE_TYPE_KERNING
};
Font::Font() {
	m_tex = 0;
}
Font::~Font() {

}
bool Font::init(ID3D11Device* device, ID3D11DeviceContext* context) {
	m_tex = new Texture();
	if (!m_tex) {
		return false;
	}
	if (!m_tex->init(device, context, m_tex_path.c_str())) {
	//if (!m_tex->init(device, context, "res/fonts/TrebuchetMS_32_2.png")) {
		return false;
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;
	D3D11_SUBRESOURCE_DATA vertex_data;
	HRESULT res;

	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(VBO) * 6;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;
	vertex_buffer_desc.StructureByteStride = 0;

	vertex_data.SysMemPitch = 0;
	vertex_data.SysMemSlicePitch = 0;

	float scale = 512.f;
	for (std::pair<char, CHARACTER> pair : m_characters) {
		CHARACTER c = pair.second;
		VBO* vertices = new VBO[6];

		vertices[0].position = DirectX::XMFLOAT3(0, -100, 1.f);
		vertices[0].uv = DirectX::XMFLOAT2(c.bounds.x / scale, 1.f - ((c.bounds.y + c.bounds.w) / scale));
		//vertices[0].uv = DirectX::XMFLOAT2(0, 0);

		vertices[1].position = DirectX::XMFLOAT3(0, 0, 1.f);
		vertices[1].uv = DirectX::XMFLOAT2(c.bounds.x / scale, 1.f - (c.bounds.y / scale));
		//vertices[1].uv = DirectX::XMFLOAT2(0, 1);

		vertices[2].position = DirectX::XMFLOAT3(100, 0, 1.f);
		vertices[2].uv = DirectX::XMFLOAT2((c.bounds.x + c.bounds.z) / scale, 1.f - (c.bounds.y / scale));
		//vertices[2].uv = DirectX::XMFLOAT2(1, 1);
			
		vertices[3].position = DirectX::XMFLOAT3(0, -100, 1.f);
		vertices[3].uv = DirectX::XMFLOAT2(c.bounds.x / scale, 1.f - ((c.bounds.y + c.bounds.w) / scale));

		vertices[4].position = DirectX::XMFLOAT3(100, 0, 1.f);
		vertices[4].uv = DirectX::XMFLOAT2((c.bounds.x + c.bounds.z) / scale, 1.f - (c.bounds.y / scale));

		vertices[5].position = DirectX::XMFLOAT3(100, -100, 1.f);
		vertices[5].uv = DirectX::XMFLOAT2((c.bounds.x + c.bounds.z) / scale, 1.f - ((c.bounds.y + c.bounds.w) / scale));

		for (int i = 0; i < 6; i++) {
			printf("u: %f, v: %f\n", vertices[i].uv.x, vertices[i].uv.y);
		}

		vertex_data.pSysMem = vertices;

		ID3D11Buffer* buffer;
		res = device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &buffer);
		if (FAILED(res)) {
			return false;
		}

		m_vertex_buffers.emplace(pair.first, buffer);

		delete[] vertices;
	}

	return true;
}
void Font::shutdown() {
	if (m_tex) {
		m_tex->shutdown();
		delete m_tex;
		m_tex = 0;
	}
	for(std::pair<char, ID3D11Buffer*> c : m_vertex_buffers){
		if (c.second) {
			c.second->Release();
			c.second = 0;
		}
	}
	m_vertex_buffers.clear();
}
bool Font::getKerningValue(char first, char second, int& ammount) {
	std::pair<char, char> pair = std::make_pair(first, second);
	if (m_kernings.find(pair) == m_kernings.end()) {
		return false;
	}
	ammount = m_kernings[pair];
	return true;
}
bool Font::getOffsetValues(char c, int& offsetX, int& offsetY, int& advance) {
	if (m_characters.find(c) == m_characters.end()) {
		return false;
	}
	offsetX = m_characters[c].offset.x;
	offsetX = m_characters[c].offset.y;
	advance = m_characters[c].advance;
}
ID3D11Buffer* Font::getCharacterBuffer(char c) {
	if (m_vertex_buffers.find(c) == m_vertex_buffers.end()) {
		return nullptr;
	}
	return m_vertex_buffers[c];
}
ID3D11ShaderResourceView* Font::getTextureResource() {
	return m_tex->getShaderResource();
}
int Font::getVBOSize() {
	return sizeof(VBO);
}
int Font::getVBOCountPerInstance() {
	return 6;
}
void Font::print() {
	printf("face: %s, charset: %s, texture: %s\n", m_face.c_str(), m_charset.c_str(), m_tex_path.c_str());
	printf("size: %d, stretchH: %d, smooth: %d, aa: %d, lineH: %d, m_base: %d, scale: %d %d\n", m_size, m_stretchH, m_smooth, m_aa, m_lineH, m_base, m_scaleW, m_scaleH);
	for (std::pair<char, CHARACTER> c : m_characters) {
		printf("character id: %c advance: %d\n", c.first, c.second.advance);
	}
	for (std::pair<std::pair<char, char>, int> c : m_kernings) {
		printf("kerning id: %c id: %c\n", c.first.first, c.first.second);
	}
}

Text::Text(Font* f, Shader* s, float x, float y) {
	m_pos.x = x;
	m_pos.y = y;

	m_font_ptr = f;
	m_shader_ptr = s;
}
Text::~Text() {

}
bool Text::init() {
	return true;
}
void Text::shutdown() {
	clear();
}
void Text::setPosition(float x, float y) {
	m_pos.x = x;
	m_pos.y = y;
}
bool Text::setText(ID3D11Device* device, std::string s) {
	clear();
	int advance = 0;
	for (int i = 0; i < s.size(); i++) {
		bool found = false;
		for (INSTANCE& inst : m_instances) {
			if (inst.character == s[i]) {
				char prev = 0;
				if(i > 0){
					prev = s[i - 1];
				}
				if (addCharToInstanceList(s[i], inst, advance, prev)) {
					printf("pos: %f, %f, %f\n", inst.ibo.back().position.x, inst.ibo.back().position.y, inst.ibo.back().position.z);
					inst.instances++;
				}
				found = true;
				break;
			}
		}
		if (!found) {
			char prev = 0;
			if (i > 0) {
				prev = s[i - 1];
			}
			INSTANCE inst;
			if (addCharToInstanceList(s[i], inst, advance, prev)) {
				inst.character = s[i];
				inst.instances++;
				printf("pos: %f, %f, %f\n", inst.ibo.back().position.x, inst.ibo.back().position.y, inst.ibo.back().position.z);
				m_instances.push_back(inst);
			}
		}
	}
	D3D11_BUFFER_DESC instance_buffer_desc;
	instance_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	instance_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instance_buffer_desc.CPUAccessFlags = 0;
	instance_buffer_desc.MiscFlags = 0;
	instance_buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA instance_data;
	instance_data.SysMemPitch = 0;
	instance_data.SysMemSlicePitch = 0;

	HRESULT res;

	for (INSTANCE& inst : m_instances) {
		instance_buffer_desc.ByteWidth = sizeof(IBO) * inst.instances;
		instance_data.pSysMem = &inst.ibo[0];

		res = device->CreateBuffer(&instance_buffer_desc, &instance_data, &inst.buffer);
		if (FAILED(res))
		{
			printf("failed to init instance buffer\n");
			return false;
		}
	}
	return true;
}
bool Text::addCharToInstanceList(char c, INSTANCE& inst, int& adv, char prev_c) {
	int kerning = 0;
	int offsetX = 0;
	int offsetY = 0;
	int next_adv = 0;
	if (m_font_ptr->getOffsetValues(c, offsetX, offsetY, next_adv)) {
		if (prev_c > 0) {
			m_font_ptr->getKerningValue(prev_c, c, kerning);
		}
		IBO ibo;
		ibo.position.x = m_pos.x + adv + kerning + offsetX;
		ibo.position.y = m_pos.y + offsetY;
		ibo.position.z = 0.f;

		inst.ibo.push_back(ibo);
		adv += next_adv;
		return true;
	}
	else {
		return false;
	}
}
void Text::clear() {
	for (INSTANCE i : m_instances) {
		if (i.buffer) {
			i.buffer->Release();
			i.buffer = 0;
		}
	}
	m_instances.clear();
}
void Text::render(ID3D11DeviceContext* context, DirectX::XMMATRIX proj) {
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	FONTBUFFER fb;
	fb.projection = proj;
	m_shader_ptr->setParams<FONTBUFFER>(context, "ft_buffer", fb);

	m_shader_ptr->setResource(PIXEL_SHADER, context, m_font_ptr->getTextureResource());

	m_shader_ptr->setActive(context);

	for (INSTANCE& inst : m_instances) {
		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = m_font_ptr->getVBOSize();
		strides[1] = sizeof(IBO);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = 0;
		bufferPointers[1] = 0;

		bufferPointers[0] = m_font_ptr->getCharacterBuffer(inst.character);
		bufferPointers[1] = inst.buffer;

		if (!bufferPointers[0]) {
			printf("vertex buffer not valid\n");
		}
		if (!bufferPointers[1]) {
			printf("instance buffer not valid\n");
		}
		//printf("character: %c, instances: %d, stride 1: %d, stride2: %d\n",inst.character, inst.instances, strides[0], strides[1]);
		context->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);

		context->DrawInstanced(m_font_ptr->getVBOCountPerInstance(), inst.instances, 0, 0);
	}
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
bool init(std::string, Font*);
bool loadFontFromFile(Font* f, std::string path) {
	bool res = init(path, f);
	f->print();
	return init;
}
bool init(std::string path, Font* f) {
	std::map<std::string, std::string> parse_info;
	std::vector< std::map<std::string, std::string> >parse_chars;
	std::vector< std::map<std::string, std::string> >parse_kerns;

	bool res = false;

	std::ifstream input(path);
	if (input.is_open()) {
		std::string line;
		while (std::getline(input, line)) {
			std::istringstream iss_line(line);
			unsigned char linetype = FILE_LINE_TYPE_UNKNOWN;
			std::string section;
			while (std::getline(iss_line, section, ' ')) {
				std::istringstream iss_section(section);
				if (linetype == FILE_LINE_TYPE_UNKNOWN) {
					if (section == "info") {
						linetype = FILE_LINE_TYPE_INFO;
					}
					else if (section == "common") {
						linetype = FILE_LINE_TYPE_COMMON;
					}
					else if (section == "page") {
						linetype = FILE_LINE_TYPE_PAGE;
					}
					else if (section == "char") {
						linetype = FILE_LINE_TYPE_CHAR;
						parse_chars.push_back(std::map<std::string, std::string>());
					}
					else if (section == "kerning") {
						linetype = FILE_LINE_TYPE_KERNING;
						parse_kerns.push_back(std::map<std::string, std::string>());
					}
					else {
						continue;
					}
				}
				else {
					std::string key;
					std::string value;
					if (std::getline(iss_section, key, '=')) {
						if (std::getline(iss_section, value)) {
							if (linetype == FILE_LINE_TYPE_INFO || linetype == FILE_LINE_TYPE_COMMON || linetype == FILE_LINE_TYPE_PAGE) {
								parse_info.emplace(key, value);
							}
							else if(linetype == FILE_LINE_TYPE_CHAR){
								parse_chars.back().emplace(key, value);
							}
							else if (linetype == FILE_LINE_TYPE_KERNING) {
								parse_kerns.back().emplace(key, value);
							}
							else {
								continue;
							}
						}
						else {
							continue;
						}
					}
					else {
						continue;
					}
				}
			}

		}
		res = setValues(f, parse_info, parse_chars, parse_kerns);
		input.close();
	}
	else {
		return false;
	}
	return res;
}
/*bool init(std::string path, Font* f) {
	std::ifstream input(path);
	std::regex r_line_headings[5] = {
		std::regex(".+"),//std::regex("(\binfo\b).+?(?=\n)"),
		std::regex("(\bcommon\b).+?(?=\n)"),
		std::regex("(\bpage\b).+?(?=\n)"),
		std::regex("(\bchar\b).+?(?=\n)"),
		std::regex("(\bkerning\b).+?(?=\n)"),
	};
	std::regex extract_data("[^\s\"\']+|\"([^\"]*)\"|\'([^\']*)\'");
	if (input.is_open()) {
		std::string line;
		while (std::getline(input, line)) {
			printf("line: %s\n", line.c_str());
			for (int i = 0; i < 5; i++) {
				if (std::regex_search(line.begin(), line.end(), r_line_headings[i])) {
					printf("match\n");
					std::vector<std::string> items = splitString(line, extract_data);
					for (std::string s : items) {
						printf("item: %f\n", s.c_str());
					}
					break;
				}
			}
		}
		input.close();
	}
	else {
		return false;
	}
}*/
bool setValues(Font* f, std::map<std::string, std::string> info, std::vector< std::map<std::string, std::string> > chars, std::vector< std::map<std::string, std::string> > kerns){
	std::string tex_path;
	for (std::pair<std::string, std::string> s : info) {
		std::string key = s.first;
		std::string value = s.second;
		if (key == "face") {
			value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
			f->m_face = value;
		}
		else if (key == "size") {
			f->m_size = std::stoi(value);
		}
		else if (key == "bold") {
			f->m_bold = std::stoi(value);
		}
		else if (key == "italic") {
			f->m_italic = std::stoi(value);
		}
		else if (key == "charset") {
			value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
			f->m_charset = value;
		}
		else if (key == "unicode") {
			f->m_unicode = std::stoi(value);
		}
		else if (key == "stretchH") {
			f->m_stretchH = std::stoi(value);
		}
		else if (key == "smooth") {
			f->m_smooth = std::stoi(value);
		}
		else if (key == "aa") {
			f->m_aa = std::stoi(value);
		}
		else if (key == "padding") {
			std::istringstream iss(value);
			std::string segment;
			int components[4];
			unsigned char counter = 0;
			while (getline(iss, segment, ',')) {
				components[counter] = std::stoi(segment);
				counter++;
			}
			f->m_padding.x = components[0];
			f->m_padding.y = components[1];
			f->m_padding.z = components[2];
			f->m_padding.w = components[3];
		}
		else if (key == "spacing") {
			std::istringstream iss(value);
			std::string segment;
			int components[2];
			unsigned char counter = 0;
			while (getline(iss, segment, ',')) {
				components[counter] = std::stoi(segment);
				counter++;
			}
			f->m_spacing.x = components[0];
			f->m_spacing.y = components[1];
		}
		else if (key == "lineHeight") {
			f->m_lineH = std::stoi(value);
		}
		else if (key == "base") {
			f->m_base = std::stoi(value);
		}
		else if (key == "scaleW") {
			f->m_scaleW = std::stoi(value);
		}
		else if (key == "scaleH") {
			f->m_scaleH = std::stoi(value);
		}
		else if (key == "file") {
			value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
			f->m_tex_path = value;
		}
	}

	for(std::map<std::string, std::string> char_line : chars){
		Font::CHARACTER c;
		char id;
		for (std::pair<std::string, std::string> s : char_line) {
			std::string key = s.first;
			std::string value = s.second;
			if (key == "id") {
				id = std::stoi(value);
			}
			if (key == "x") {
				c.bounds.x = std::stoi(value);
			}
			else if (key == "y") {
				c.bounds.y = std::stoi(value);
			}
			else if (key == "width") {
				c.bounds.z = std::stoi(value);
			}
			else if (key == "height") {
				c.bounds.w = std::stoi(value);
			}
			else if (key == "xoffset") {
				c.offset.x = std::stoi(value);
			}
			else if (key == "yoffset") {
				c.offset.y = std::stoi(value);
			}
			else if (key == "xadvance") {
				c.advance = std::stoi(value);
			}
		}
		f->m_characters.emplace(id, c);
	}

	for (std::map<std::string, std::string> kern_line : kerns) {
		char first, second;
		int ammount;
		for (std::pair<std::string, std::string> s : kern_line) {
			std::string key = s.first;
			std::string value = s.second;
			if (key == "first") {
				first = std::stoi(value);
			}
			else if (key == "second") {
				second = std::stoi(value);
			}
			else if (key == "ammount") {
				ammount = std::stoi(value);
			}
		}
		f->m_kernings.emplace(std::make_pair(first, second), ammount);
	}

	return true;
}
