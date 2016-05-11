/*
 * =====================================================================================
 *
 *       Filename:  client.h
 *
 *    Description:  Structure which holds info about client connections.
 *
 *         Author:  Jes Tikalsky, Evan Friis (UW Madison)
 *
 * =====================================================================================
 */

#ifndef IPBUS_CLIENT_H
#define IPBUS_CLIENT_H

#include "bytebuffer.h"
#include "circular_buffer.h"

typedef struct {
  // Buffers for I/O with internal IPBus handling algorithms
  CircularBuffer* inputstream;
  CircularBuffer* outputstream;
  // The I/O buffers are 32bit.  This buffer is a shim to ensure we always
  // pass along 32bit chunks, since we might get less than a full word over TCP.
  // In the future, this could also be circular.
  ByteBuffer byte2word;
  // File descriptors for communication with outside world
  int inputfd;
  int outputfd;
  // Whether or not this client has the same endianness as the target
  int swapbytes;
} Client;

#endif
