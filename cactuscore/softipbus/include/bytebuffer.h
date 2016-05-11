/*
 * =====================================================================================
 *
 *       Filename:  bytebuffer.h
 *
 *    Description:  A generic buffer of bytes & memory management helpers.
 *
 *         Author:  Jes Tikalsky, Evan Friis, UWMadison
 *
 * =====================================================================================
 */

#ifndef IPBUS_byteBUFFER_H
#define IPBUS_byteBUFFER_H

#include <stddef.h>
#include <sys/types.h>

typedef struct {
    int bufsize;
    unsigned char *buf;
} ByteBuffer;

// Build a buffer. If data is NULL, it will be a set
// of n bytes initialized to zero.
ByteBuffer bytebuffer_ctor(const unsigned char* data, size_t n);

// remove memory allocated to a buffer
void bytebuffer_free(ByteBuffer* buffer);

// Add n bytes of capacity (uninitialized) to the end of the buffer.
void bytebuffer_reserve_back(ByteBuffer* buffer, size_t n);

// Append data to a buffer.  If the buffer is not allocated (buf == NULL),
// this call functions equivalently to bytebuffer_ctor.  If data is NULL,
// the buffer will be padded with zeros.
void bytebuffer_append(ByteBuffer* buffer, const unsigned char* data, size_t n);

// Pop data from front of buffer.  The popped data is returned as a new buffer.
// If n is greater than the length of the buffer, the entire buffer is popped.
ByteBuffer bytebuffer_pop(ByteBuffer* buffer, size_t n);

// delete data from front of buffer
void bytebuffer_del_front(ByteBuffer* buffer, size_t n);

// delete data from back of buffer
void bytebuffer_del_back(ByteBuffer* buffer, size_t n);

// Read a file descriptor, appending into a buffer.
ssize_t bytebuffer_read_fd(ByteBuffer* buffer, int fd, size_t n);

#endif
