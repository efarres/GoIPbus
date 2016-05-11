/*
 * =====================================================================================
 *
 *       Filename:  ipbus.h
 *
 *    Description:  Functions for encoding and decoding IPBus packets.
 *
 *         Author:  Jes Tikalsky, Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#ifndef IPBUS_SERIALIZATION_H
#define IPBUS_SERIALIZATION_H

#include <stdint.h>
#include "protocol.h"
#include "bytebuffer.h"
#include "circular_buffer.h"

// Looks at the head of a stream and decides if we can process something.
// Returns an IPBUS_ISTREAM_XXX flag.  If a IPBus packet header is detected,
// it will set the value of <swapbytes> according to whether the client
// has same or opposite endianness.
int ipbus_stream_state(const CircularBuffer* input_buffer, int* swapbytes);

// Detect if a word is a packet header.  Returns 0 if not,
// IPBUS_ISTREAM_PACKET if yes using native endianness,
// and IPBUS_ISTREAM_PACKET_SWP_ORD if using opposite endianness
int ipbus_detect_packet_header(uint32_t word);

// Initialize an IPbus transaction from a header word.  Does not
// allocate any memory for the payload (see ipbus_decode_transaction).
ipbus_transaction_t ipbus_decode_transaction_header(const CircularBuffer* buf, int swapbytes);

// Decode an IPBus transaction from a buffer. <swapbytes> determines if the
// endianness of the stream is different than the local.
ipbus_transaction_t ipbus_decode_transaction(const CircularBuffer* buf, int swapbytes);

// Determine the size (in words) of a transaction payload (everything besides header).
size_t ipbus_transaction_payload_size(
    unsigned char words, unsigned char type, unsigned char info_code);

// Determine the size of an encoded transaction packet (in words)
size_t ipbus_transaction_endocded_size(const ipbus_transaction_t* transaction);

// Encode a packet into a string
void ipbus_encode_transaction(CircularBuffer* into, const ipbus_transaction_t* t, int swapbytes);

// Build an IPBus transaction header
uint32_t ipbus_transaction_header(
    uint32_t protocol,
    uint32_t transaction_id,
    uint32_t words,
    uint32_t type_id,
    uint32_t info_code);

// Build an IPBus v2 packet header
uint32_t ipbus_packet_header(uint32_t packet_id, uint32_t type);

#endif
