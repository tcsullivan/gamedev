#ifndef BMP_IMAGE_HPP
#define BMP_IMAGE_HPP

#include <cstdint>

/**
 * Defines the layout of a bitmap (.bmp) file's header.
 */
typedef struct {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

/**
 * Defines the layout of a bitmap's info header.
 */
typedef struct {
	uint32_t biSize;
	 int32_t biWidth;
	 int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	 int32_t biXPelsPerMeter;
	 int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} __attribute__((packed)) BITMAPINFOHEADER;

#endif // BMP_IMAGE_HPP
