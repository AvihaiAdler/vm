#include "../include/image_loader.h"

#include <stdint.h>

uint16_t swap_endianess(uint16_t data) {
  return (uint16_t)((data >> 8) | (data << 8));
}

bool read_image_file(FILE* image, uint16_t* const memory) {
  // the starting memory address at which we load our program into
  uint16_t origin;
  if (!fread(&origin, sizeof origin, 1, image)) {
    return false;
  }

  origin = swap_endianess(origin);

  // since our memory address can't be higher than UINT16_MAX we can calculate
  // how much we need to read from the image
  uint16_t max_read = (uint16_t)(UINT16_MAX - origin);
  size_t read = fread(memory + origin, sizeof(uint16_t), max_read, image);
  if (!read) {
    return false;
  }

  while(read > 0) {
    memory[origin] = swap_endianess(memory[origin]);
    origin++;
    read--;
  }
  return true;
}

bool read_image(const char* image_path, uint16_t* const memory) {
  FILE* image_stream = fopen(image_path, "rb");
  if (!image_stream) {
    fprintf(stderr, "failed to open file %s\n", image_path);
    return false;
  }

  if (!read_image_file(image_stream, memory)) return false;
  fclose(image_stream);
  return true;
}
