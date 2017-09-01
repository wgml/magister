#include "dump_png.h"

// source: http://www.labbookpages.co.uk/software/imgProc/files/libPNG/makePNG.c

static void gbr2rgb(png_bytep buffer, int pixels) {
	unsigned int pixel_size = 3;
	png_byte tmp[pixel_size];
	for (unsigned int i = 0; i < pixels * pixel_size; i += pixel_size) {
		memcpy(tmp, buffer + i, pixel_size);
		buffer[i] = tmp[2];
		buffer[i + 1] = tmp[0];
		buffer[i + 2] = tmp[1];
	}
}

static int writeImage(const char* filename, int width, int height, unsigned char *buffer, char* title)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;

	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "Error during png creation\n");
		code = 1;
		goto finalise;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Set title
	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = (char *) "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	row = (png_bytep) malloc(3 * width * sizeof(png_byte));

	// Write image data
	int y;
	for (y=0 ; y<height ; y++) {
		memcpy(row, buffer + 3 * y * width, 3 * width);
		gbr2rgb(row, width);
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return code;
}


int dump_png_to_file(const char *filename, struct video_config config, axi_vdma vdma) {
	long buffer_size = config.video.width * config.video.height * config.video.pixel_size;
	unsigned char buffer[buffer_size];

	size_t index = vdma.current_framebuffer_index;
	memcpy(buffer, vdma.framebuffers[index].virtual_addr, buffer_size);

	return dump_buffer_to_file(filename, buffer, config.video.width, config.video.height);
}

int dump_buffer_to_file(const char *filename, unsigned char *buffer, int width, int height) {
	return writeImage(filename, width, height, buffer, (char *)filename);
}
