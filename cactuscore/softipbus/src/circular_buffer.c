#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "endiantools.h"
#include "circular_buffer.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

CircularBuffer* cbuffer_new(void) {
  CircularBuffer* output = (CircularBuffer*)malloc(sizeof(CircularBuffer));
  output->data = (uint32_t*)calloc(IO_BUFFER_SIZE, sizeof(uint32_t));
  output->tail = 0;
  output->pos = 0;
  return output;
}

CircularBuffer* cbuffer_copy(CircularBuffer* from) {
  CircularBuffer* output = (CircularBuffer*)malloc(sizeof(CircularBuffer));
  output->data = (uint32_t*)malloc(IO_BUFFER_SIZE * sizeof(uint32_t));
  memcpy(output->data, from->data, IO_BUFFER_SIZE * sizeof(uint32_t));
  output->tail = from->tail;
  output->pos = from->pos;
  return output;
}

uint32_t cbuffer_size(const CircularBuffer* buffer) {
  if (buffer->pos <= buffer->tail) 
    return buffer->tail - buffer->pos;
  return buffer->tail + IO_BUFFER_SIZE - buffer->pos;
}

uint32_t cbuffer_contiguous_data_size(const CircularBuffer* buffer) {
  if (buffer->pos <= buffer->tail) 
    return buffer->tail - buffer->pos;
  return IO_BUFFER_SIZE - buffer->pos;
}

void cbuffer_free(CircularBuffer* tokill) {
  free(tokill->data);
  free(tokill);
}

uint32_t cbuffer_value_at(const CircularBuffer* buf, uint32_t idx) {
  uint32_t actual_idx = (buf->pos + idx) % IO_BUFFER_SIZE;
  return buf->data[actual_idx];
}

uint32_t cbuffer_value_at_net(const CircularBuffer* buf, uint32_t idx) {
  return network_to_host(cbuffer_value_at(buf, idx));
}

uint32_t cbuffer_freespace(const CircularBuffer* buf) {
  return IO_BUFFER_SIZE - cbuffer_size(buf) - 1;
}

int cbuffer_append(CircularBuffer* buffer, void* data, uint32_t nwords) {
  uint32_t freespace = cbuffer_freespace(buffer);
  if (freespace < nwords) {
    return -1;
  }

  // words available on "tail" until we hit the absolute end of the memory.
  // we know from the freespace check that we can't overwrite the head with
  // only <nwords>.
  uint32_t tail_length = IO_BUFFER_SIZE - buffer->tail;

  memcpy(
      &(buffer->data[buffer->tail]),
      data, 
      sizeof(uint32_t) * min(nwords, tail_length));

  // if we didn't write everything, we need to wrap around to the beginning.
  if (tail_length < nwords) {
    memcpy(
        buffer->data, 
        (uint8_t*)data + sizeof(uint32_t) * tail_length,
        sizeof(uint32_t) * (nwords - tail_length));
  }
  buffer->tail = (buffer->tail + nwords) % IO_BUFFER_SIZE;
  return 0;
}

int cbuffer_push_back(CircularBuffer* buffer, uint32_t data) {
  uint32_t freespace = cbuffer_freespace(buffer);
  if (freespace < 1) {
    return -1;
  }
  buffer->data[buffer->tail] = data;
  buffer->tail = ((uint32_t)(buffer->tail + 1)) % IO_BUFFER_SIZE;
  return 0;
}

int cbuffer_push_back_net(CircularBuffer* buffer, uint32_t data) {
  return cbuffer_push_back(buffer, host_to_network(data));
}

uint32_t cbuffer_read(const CircularBuffer* buffer, uint32_t* output, uint32_t nwords) {
  uint32_t words_to_read = min(nwords, cbuffer_size(buffer));
  uint32_t tail_words_to_read = min(words_to_read, cbuffer_contiguous_data_size(buffer));
  memcpy(
      output,
      &(buffer->data[buffer->pos]),
      sizeof(uint32_t) * tail_words_to_read);
  // check if we need to wrap around.
  uint32_t remaining_words_at_head = words_to_read - tail_words_to_read;
  if (remaining_words_at_head) {
    memcpy(
        &(output[tail_words_to_read]), 
        buffer->data, 
        sizeof(uint32_t) * remaining_words_at_head);
  }
  return words_to_read;
}

uint32_t cbuffer_deletefront(CircularBuffer* buffer, uint32_t nwords) {
  uint32_t words_to_delete = min(nwords, cbuffer_size(buffer));
  buffer->pos += words_to_delete;
  buffer->pos %= IO_BUFFER_SIZE;
  return words_to_delete;
}

Buffer* cbuffer_pop(CircularBuffer* buffer, uint32_t nwords) {
  Buffer* output = buffer_new(NULL, nwords);
  uint32_t actually_read = cbuffer_read(buffer, output->data, nwords);
  buffer_resize(output, actually_read);
  cbuffer_deletefront(buffer, nwords);
  return output;
}

uint32_t cbuffer_pop_front(CircularBuffer* buffer) {
  if (!cbuffer_size(buffer)) 
    return 0xDEADBEEF;
  uint32_t output = cbuffer_value_at(buffer, 0);
  cbuffer_deletefront(buffer, 1);
  return output;
}

ssize_t cbuffer_write_fd(CircularBuffer* buffer, int fd, size_t nwords) {
  if (!nwords) {
    return 0;
  }
  uint32_t words_to_write = min(nwords, cbuffer_size(buffer));
  uint32_t tail_words_to_read = min(words_to_write, cbuffer_contiguous_data_size(buffer));
  ssize_t written = write(fd, &(buffer->data[buffer->pos]), 
      tail_words_to_read * sizeof(uint32_t)) / sizeof(uint32_t);
  cbuffer_deletefront(buffer, written);
  if (written == 0)
    return 0;
  return written + cbuffer_write_fd(buffer, fd, nwords - written);
}

ssize_t cbuffer_read_fd(CircularBuffer* buffer, int fd, size_t nwords) {
  if (!nwords) {
    return 0;
  }
  uint32_t freespace = cbuffer_freespace(buffer);
  size_t words_to_read = min(nwords, freespace);

  // words available on "tail" until we hit the absolute end of the memory.
  // we know from the freespace check that we can't overwrite the head with
  // only <nwords>.
  words_to_read = min(IO_BUFFER_SIZE - buffer->tail, words_to_read);

  ssize_t words_read = read(fd, &(buffer->data[buffer->tail]), 
      words_to_read * sizeof(uint32_t)) / sizeof(uint32_t);

  buffer->tail = (buffer->tail + words_read) % IO_BUFFER_SIZE;

  if (words_read == 0) {
    return 0;
  } 
  return words_read + cbuffer_read_fd(buffer, fd, nwords - words_read);
}

uint32_t cbuffer_transfer_data(CircularBuffer* source, CircularBuffer* destination) {
  // check for NULL pointers
  if (!source || !destination) {
    return -1;
  }
  uint32_t source_data_size = cbuffer_size(source);
  uint32_t dest_free_space  = cbuffer_freespace(destination);

  uint32_t words2transfer = min(source_data_size, dest_free_space);

  Buffer* data2transfer = cbuffer_pop(source, words2transfer);
  cbuffer_append(destination, data2transfer->data, words2transfer);
  
  buffer_free(data2transfer); /* release memory of intermediate Buffer */

  return words2transfer;
}
