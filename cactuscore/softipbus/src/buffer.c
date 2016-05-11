#include <stdlib.h>
#include <string.h>

#include "buffer.h"

Buffer* buffer_new(void* data, uint32_t size) {
  Buffer* output = (Buffer*)malloc(sizeof(Buffer));
  output->data = (uint32_t*)malloc(size * sizeof(uint32_t));
  output->size = size;
  if (data) {
    memcpy(output->data, data, size * sizeof(uint32_t));
  } else {
    memset(output->data, 0, size * sizeof(uint32_t));
  }
  return output;
}

void buffer_free(Buffer* buf) {
  free(buf->data);
  free(buf);
}

void buffer_resize(Buffer* buf, uint32_t size) {
  buf->data = (uint32_t*)realloc(buf->data, size * sizeof(uint32_t));
  buf->size = size;
}
