#ifndef IMAGE_LOADER_H_
#define IMAGE_LOADER_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool read_image_file(FILE* image, uint16_t* const memory);

bool read_image(const char* image_path, uint16_t* const memory);

#endif
