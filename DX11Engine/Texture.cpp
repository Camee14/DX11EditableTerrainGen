#include "Texture.h"

#include "FreeImage.h"

Texture::Texture() {

}
Texture::Texture(const Texture& other) {

}
Texture::~Texture() {

}
bool Texture::init(ID3D11Device* device, ID3D11DeviceContext* context, const char* path) {
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* dib(0);
	BYTE* bits(0);

	unsigned int width(0), height(0), BPP(0);

	bool res = false;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(path, 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilename(path);
	}
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN) {
		return false;
	}

	//check that the plugin has reading capabilities and load the file
	if (!FreeImage_FIFSupportsReading(fif)) {
		return false;
	}
	dib = FreeImage_Load(fif, path);

	//if the image failed to load, return failure
	if (!dib) {
		return false;
	}

	//make sure format is 32 bit for directX
	BPP = FreeImage_GetBPP(dib);
	if (BPP == 24) {
		dib = FreeImage_ConvertTo32Bits(dib);
	}

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	BPP = FreeImage_GetBPP(dib);

	if ((bits == 0) || (width == 0) || (height == 0) || (BPP != 32)) {
		return false;
	}

	printf("succesfully loaded file with info: width = %d, height = %d\n", width, height);

	D3D11_TEXTURE2D_DESC tex_desc;
	//ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC)); nor sure if nessesary
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	res = device->CreateTexture2D(&tex_desc, nullptr, &m_texture);
	if (FAILED(res)) {
		return false;
	}

	unsigned int row_size = width * (BPP / 8);

	context->UpdateSubresource(m_texture, 0, NULL, bits, row_size, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 8;

	res = device->CreateShaderResourceView(m_texture, &srv_desc, &m_texture_view);
	if (FAILED(res)) {
		return false;
	}

	context->GenerateMips(m_texture_view);

	FreeImage_Unload(dib);
}
void Texture::shutdown() {
	if (m_texture_view) {
		m_texture_view->Release();
		m_texture_view = 0;
	}
	if (m_texture) {
		m_texture->Release();
		m_texture = 0;
	}
}
ID3D11ShaderResourceView* Texture::getShaderResource() {
	return m_texture_view;
}