#include "bytebuffer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ByteBuffer bytebuffer_ctor(const unsigned char* data, size_t n) {
  ByteBuffer output;
  output.bufsize = n;
  output.buf = NULL;
  if (n) {
    if (data != NULL) {
      output.buf = (unsigned char*) malloc(n);
      memcpy(output.buf, data, n);
    } else {
      output.buf = (unsigned char*)calloc(n, 1);
    }
  }
  return output;
}

void bytebuffer_free(ByteBuffer* buffer) {
  free(buffer->buf);
  buffer->bufsize = 0;
  buffer->buf = NULL;
}

void bytebuffer_reserve_back(ByteBuffer* buffer, size_t n) {
  // From: http://www.cplusplus.com/reference/cstdlib/realloc/
  // "Alternatively, this can be a null pointer, in which case a new block is
  // allocated (as if malloc was called)."
  buffer->buf = (unsigned char*)realloc(buffer->buf, buffer->bufsize + n);
  buffer->bufsize += n;
}

void bytebuffer_append(ByteBuffer* buffer, const unsigned char* data, size_t n) {
  size_t original_size = buffer->bufsize;
  bytebuffer_reserve_back(buffer, n);
  if (data != NULL) {
    memcpy(buffer->buf + original_size, data, n);
  } else {
    memset(buffer->buf + original_size, 0, n);
  }
}

void bytebuffer_del_back(ByteBuffer* buffer, size_t n) {
  if ((int)n >= buffer->bufsize) {
    bytebuffer_free(buffer);
  } else {
    buffer->bufsize -= n;
    buffer->buf = (unsigned char*)realloc(buffer->buf, buffer->bufsize);
  }
}

void bytebuffer_del_front(ByteBuffer* buffer, size_t n) {
  if ((int)n >= buffer->bufsize) {
    bytebuffer_free(buffer);
  } else {
    // shift original data back to front of buffer
    memmove(buffer->buf, buffer->buf + n, buffer->bufsize - n);
    bytebuffer_del_back(buffer, n);
  }
}

ByteBuffer bytebuffer_pop(ByteBuffer* buffer, size_t n) {
  ByteBuffer output;
  // the requested amount is greater than what we have, pop everything.
  if ((int)n >= buffer->bufsize) {
    output.bufsize = buffer->bufsize;
    output.buf = (unsigned char*)malloc(buffer->bufsize);
    memcpy(output.buf, buffer->buf, buffer->bufsize);
    bytebuffer_free(buffer);
    return output;
  }
  output.buf = (unsigned char*)malloc(n);
  output.bufsize = n;
  memcpy(output.buf, buffer->buf, n);
  bytebuffer_del_front(buffer, n);
  return output;
}

ssize_t bytebuffer_read_fd(ByteBuffer* buffer, int fd, size_t n) {
  size_t original_size = buffer->bufsize;
  bytebuffer_reserve_back(buffer, n);
  ssize_t bytes_read = read(fd, &(buffer->buf[original_size]), n);
  // Unallocate any leftover room at the end of the buffer.
  // The check is to protect against the bytes_read = -1 on error
  // case.
  size_t deallocate = bytes_read > 0 ? n - bytes_read : n;
  bytebuffer_del_back(buffer, deallocate);
  return bytes_read;
}
