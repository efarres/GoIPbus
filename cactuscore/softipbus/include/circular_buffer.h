/*
 * ============================================================================
 *
 *       Filename:  circular_buffer.h
 *
 *    Description:  A circular buffer of words.
 *
 *         Author:  Evan Friis, evan.friis@cern.ch
 *        Company:  UW Madison
 *
 * ============================================================================
 */

#ifndef CIRCULAR_BUFFER_SKY7YN38
#define CIRCULAR_BUFFER_SKY7YN38

//#define IO_BUFFER_SIZE (2048*1024)
#define IO_BUFFER_SIZE 256

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include "buffer.h"

typedef struct {
  uint32_t* data;
  uint32_t tail;
  uint32_t pos; // position w.r.t. <data> pointer where data starts.
} CircularBuffer;

// Build a new circular buffer, initialized to zero
CircularBuffer* cbuffer_new(void);

// Make a copy of a circular buffer
CircularBuffer* cbuffer_copy(CircularBuffer* from);

// Words in the buffer
uint32_t cbuffer_size(const CircularBuffer* buffer);

// Contiguous data at the head (i.e. everything before it wraps)
uint32_t cbuffer_contiguous_data_size(const CircularBuffer* buffer);

// Free memory associated to a circular buffer
void cbuffer_free(CircularBuffer*);

// Get the word at a given index
uint32_t cbuffer_value_at(const CircularBuffer* buf, uint32_t idx);

// Get the word at a given index, assuming network endianness
uint32_t cbuffer_value_at_net(const CircularBuffer* buf, uint32_t idx);

// Check remaining space in the buffer
uint32_t cbuffer_freespace(const CircularBuffer*);

// Append <nwords> from <data> to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_append(CircularBuffer* buffer, void* data, uint32_t nwords);

// Append 1 word to the buffer.
// Returns 0 on success, -1 if there is not enough room.  In this case,
// no data is added to the buffer.
int cbuffer_push_back(CircularBuffer* buffer, uint32_t data);

// As above, but respects network endianness
int cbuffer_push_back_net(CircularBuffer* buffer, uint32_t data);

// Read (up to) nwords from the buffer.  Returns words read.
uint32_t cbuffer_read(const CircularBuffer* buffer, uint32_t* dest, uint32_t nwords);

// Read (up to) nwords from the buffer.  Returns number of bytes deleted.
uint32_t cbuffer_deletefront(CircularBuffer* buffer, uint32_t nwords);

// Pop (up to) nwords from the buffer.  Equivalent to read + deletefront.
Buffer* cbuffer_pop(CircularBuffer* buffer, uint32_t nwords);

// Pop one word from the buffer.  Equivalent to value_at(0) + deletefront(1).
// The caller is responsible for checking the buffer isn't empty.
// If there is no data in the buffer, it will return 0xDEADBEEF.
uint32_t cbuffer_pop_front(CircularBuffer* buffer);

// Write data from buffer to FD.  Buffer is consumed.
ssize_t cbuffer_write_fd(CircularBuffer* buffer, int fd, size_t nwords);

// Write data from FD into buffer.
ssize_t cbuffer_read_fd(CircularBuffer* buffer, int fd, size_t nwords);

// Transfer as much data as possible from source to destination cbuffers
// Return the number of words transferred.
uint32_t cbuffer_transfer_data(CircularBuffer* source, CircularBuffer* destination);

#endif
