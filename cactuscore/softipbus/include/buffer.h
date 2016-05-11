/*
 * =====================================================================================
 *
 *       Filename:  buffer.h
 *
 *    Description:  A linear buffer of words.
 *
 *        Authors:  Evan Friis, evan.friis@cern.ch
 *                  Jes Tikalsky
 *        Company:  UW Madison
 *
 * =====================================================================================
 */

#ifndef BUFFER_2Y0RCUIN
#define BUFFER_2Y0RCUIN

#include <stddef.h>
#include <stdint.h>

// A buffer of words
typedef struct {
  uint32_t* data;
  uint32_t size;
} Buffer;

// Build a new buffer, from data. If data is 0, it will be initialized
// to zero.
Buffer* buffer_new(void* data, uint32_t size);

// Free memory associated to a buffer
void buffer_free(Buffer*);

// Resize a buffer.
void buffer_resize(Buffer*, uint32_t size);

// Append data to a buffer.  If the buffer is not allocated (buf == NULL),
// this call functions equivalently to buffer_ctor.  If data is NULL,
// the buffer will be padded with zeros.
void buffer_append(Buffer* buffer, uint32_t* data, size_t nwords);

// Pop data from front of buffer.  The popped data is returned as a new buffer.
// If n is greater than the length of the buffer, the entire buffer is popped.
Buffer buffer_pop(Buffer* buffer, size_t n);

// delete data from front of buffer
void buffer_del_front(Buffer* buffer, size_t n);

// delete data from back of buffer
void buffer_del_back(Buffer* buffer, size_t n);

#endif /* end of include guard: BUFFER_2Y0RCUIN */
