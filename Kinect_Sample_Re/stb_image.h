#pragma once

typedef unsigned char stbi_uc;

stbi_uc* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp);
void stbi_image_free(void* retval_from_stbi_load);

#ifdef STB_IMAGE_IMPLEMENTATION

#include <windows.h>
#include <wincodec.h>
#include <stdlib.h>

static wchar_t* stbi__to_wide_path(const char* filename)
{
	int length = MultiByteToWideChar(CP_ACP, 0, filename, -1, NULL, 0);
	if (length <= 0)
		return NULL;

	wchar_t* wide = (wchar_t*)malloc(sizeof(wchar_t) * length);
	if (wide == NULL)
		return NULL;

	MultiByteToWideChar(CP_ACP, 0, filename, -1, wide, length);
	return wide;
}

stbi_uc* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp)
{
	IWICImagingFactory* factory = NULL;
	IWICBitmapDecoder* decoder = NULL;
	IWICBitmapFrameDecode* frame = NULL;
	IWICFormatConverter* converter = NULL;
	wchar_t* widePath = NULL;
	stbi_uc* pixels = NULL;
	stbi_uc* convertedPixels = NULL;
	UINT width = 0;
	UINT height = 0;
	HRESULT hr;

	if (x != NULL) *x = 0;
	if (y != NULL) *y = 0;
	if (comp != NULL) *comp = 0;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
		return NULL;

	widePath = stbi__to_wide_path(filename);
	if (widePath == NULL)
		return NULL;

	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&factory));
	if (FAILED(hr))
		goto cleanup;

	hr = factory->CreateDecoderFromFilename(widePath, NULL, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr))
		goto cleanup;

	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr))
		goto cleanup;

	hr = frame->GetSize(&width, &height);
	if (FAILED(hr) || width == 0 || height == 0)
		goto cleanup;

	hr = factory->CreateFormatConverter(&converter);
	if (FAILED(hr))
		goto cleanup;

	hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))
		goto cleanup;

	pixels = (stbi_uc*)malloc(width * height * 4);
	if (pixels == NULL)
		goto cleanup;

	hr = converter->CopyPixels(NULL, width * 4, width * height * 4, pixels);
	if (FAILED(hr))
		goto cleanup;

	if (x != NULL) *x = (int)width;
	if (y != NULL) *y = (int)height;
	if (comp != NULL) *comp = 4;

	if (req_comp == 3)
	{
		convertedPixels = (stbi_uc*)malloc(width * height * 3);
		if (convertedPixels == NULL)
			goto cleanup;

		for (UINT i = 0; i < width * height; i++)
		{
			convertedPixels[i * 3 + 0] = pixels[i * 4 + 0];
			convertedPixels[i * 3 + 1] = pixels[i * 4 + 1];
			convertedPixels[i * 3 + 2] = pixels[i * 4 + 2];
		}
		free(pixels);
		pixels = convertedPixels;
		convertedPixels = NULL;
		if (comp != NULL) *comp = 3;
	}
	else if (req_comp == 1)
	{
		convertedPixels = (stbi_uc*)malloc(width * height);
		if (convertedPixels == NULL)
			goto cleanup;

		for (UINT i = 0; i < width * height; i++)
			convertedPixels[i] = (stbi_uc)((pixels[i * 4 + 0] + pixels[i * 4 + 1] + pixels[i * 4 + 2]) / 3);

		free(pixels);
		pixels = convertedPixels;
		convertedPixels = NULL;
		if (comp != NULL) *comp = 1;
	}

cleanup:
	if (FAILED(hr))
	{
		if (pixels != NULL)
		{
			free(pixels);
			pixels = NULL;
		}
	}
	if (convertedPixels != NULL) free(convertedPixels);
	if (converter != NULL) converter->Release();
	if (frame != NULL) frame->Release();
	if (decoder != NULL) decoder->Release();
	if (factory != NULL) factory->Release();
	if (widePath != NULL) free(widePath);
	return pixels;
}

void stbi_image_free(void* retval_from_stbi_load)
{
	free(retval_from_stbi_load);
}

#endif
