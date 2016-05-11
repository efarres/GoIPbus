#include "serialization.h"

#include "endiantools.h"
#include "macrologger.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

size_t ipbus_transaction_payload_size(unsigned char words, unsigned char type, unsigned char info_code) {
  size_t datasize = 0;
  if (info_code != IPBUS_INFO_REQUEST && info_code != IPBUS_INFO_SUCCESS) {
    // this is an error, w/ no data returned.
    datasize = 0;
  } else {
    int is_response = (info_code == IPBUS_INFO_SUCCESS);
    switch (type) {
      case IPBUS_READ:
      case IPBUS_NIREAD:
        // request = 1 word with addr to read
        // response = n words read starting from addr
        datasize = is_response ? words : 1;
        break;
      case IPBUS_WRITE:
      case IPBUS_NIWRITE:
        // request = base addr + n words to write at addr
        // response = nothing
        datasize = is_response ? 0 : words + 1;
        break;
      case IPBUS_RMW:
        // request = addr, AND term, OR term
        // response = contentes of addr before modify/write
        datasize = is_response ? 1 : 3;
        break;
      case IPBUS_RMWSUM:
        // request = addr, addend
        // response = contentes of addr before modify/write
        datasize = is_response ? 1 : 2;
        break;
    }
  }
  return datasize;
}

int ipbus_detect_packet_header(uint32_t headerword) {
  // check endianess.  
  uint32_t msnibble = headerword & 0xf0000000;
  uint32_t lsnibble = headerword & 0xf0;

  if (lsnibble == 0x20 && msnibble == 0xf0000000) {
    // we need to swap the endianness
    return 2;
  } else if (lsnibble != 0xf0 || msnibble != 0x20000000) {
    return 0;
  }
  return 1;
}

int ipbus_stream_state(const CircularBuffer* input_buffer, int* swapbytes) {
  if (!cbuffer_size(input_buffer)) {
    return IPBUS_ISTREAM_EMPTY;
  }
  uint32_t firstword = cbuffer_value_at_net(input_buffer, 0);
  //LOG_DEBUG("Got a word: %"PRIx32, firstword);
  // check if this is a packet header
  int is_pkt = ipbus_detect_packet_header(firstword);
  if (is_pkt) {
    // check if we want to update an endianness flag
    if (swapbytes != NULL) {
      if (is_pkt == IPBUS_ISTREAM_PACKET)
        *swapbytes = 0;
      if (is_pkt == IPBUS_ISTREAM_PACKET_SWP_ORD)
        *swapbytes = 1;
    }
    return is_pkt;
  }
  // Double check if it is reasonable transaction packet
  // header.  We should never have the middle of a transaction
  // at the header of the buffer - we always wait for a 
  // transaction to be in-buffer before reading it out.
  ipbus_transaction_t transaction = ipbus_decode_transaction_header(
      input_buffer, *swapbytes);

  if ((int)cbuffer_size(input_buffer) >= (transaction.data.size + 1)) {
    return IPBUS_ISTREAM_FULL_TRANS;
  }

  //LOG_DEBUG("Partial transaction size: %"PRIx32, cbuffer_size(input_buffer));
  return IPBUS_ISTREAM_PARTIAL_TRANS;
}

// initialize IPbus transaction from a stream
ipbus_transaction_t ipbus_decode_transaction_header(const CircularBuffer* buf, int swapbytes) {
  ipbus_transaction_t output;

  uint32_t headerword = cbuffer_value_at_net(buf, 0);

  if (swapbytes)
    headerword = __bswap_32(headerword);

  output.info = headerword & 0x0f;
  output.type = (headerword & 0xf0) >> 4;
  output.words = (headerword & 0xff00) >> 8;
  output.id = (headerword & 0x0fff0000) >> 16;

  // determine payload size
  output.data.size = ipbus_transaction_payload_size(
      output.words, output.type, output.info);

  output.data.words = NULL;

  return output;
}

// read an IPbus transaction from a stream
ipbus_transaction_t ipbus_decode_transaction(const CircularBuffer *buf, int swapbytes) {
  ipbus_transaction_t output = ipbus_decode_transaction_header(buf, swapbytes);

  size_t words_read = 1;

  output.data.words = (uint32_t*)malloc(output.data.size * sizeof(uint32_t));

  for (unsigned int i = 0; i < output.data.size; ++i) {
    uint32_t dataword = network_to_host(cbuffer_value_at(buf, words_read)); 
    words_read += 1;
    if (swapbytes) {
      dataword = __bswap_32(dataword);
    }
    output.data.words[i] = dataword;
  }
  return output;
}

size_t ipbus_transaction_endocded_size(const ipbus_transaction_t* trans) {
  return (1 + ipbus_transaction_payload_size(trans->words, trans->type, trans->info)); 
}

uint32_t ipbus_transaction_header(
    uint32_t protocol,
    uint32_t transaction_id,
    uint32_t words,
    uint32_t type_id,
    uint32_t info_code) {
  uint32_t output = 
      (protocol << 28) |
      ((transaction_id & 0xfff) << 16) |
      ((words & 0xff) << 8) |
      ((type_id & 0xf) << 4) |
      (info_code & 0x0f);
  return output;
}

void ipbus_encode_transaction(CircularBuffer* into, const ipbus_transaction_t* transaction, int swapbytes) {
  uint32_t headerword = ipbus_transaction_header(
      2, transaction->id, transaction->words, transaction->type, transaction->info);
  if (swapbytes) {
    headerword = __bswap_32(headerword);
  }
  cbuffer_push_back_net(into, headerword);
  for (size_t i = 0; i < transaction->data.size; ++i) {
    uint32_t datum = transaction->data.words[i];
    if (swapbytes) {
      datum = __bswap_32(datum);
    }
    cbuffer_push_back_net(into, datum);
  }
}

uint32_t ipbus_packet_header(uint32_t packet_id, uint32_t type) {
  return 0x200000f0 | ((0xffff & packet_id) << 8) | (0xf & type);
}
