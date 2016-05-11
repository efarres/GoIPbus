/*
 * =====================================================================================
 *
 *       Filename:  test_ipbus.c
 *
 *    Description:  Tests of serialization functionality in ipbus.h
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include "endiantools.h"
#include "serialization.h"
#include "minunit.h"

// Test building transaction header
static char* test_ipbus_transaction_header(void) {
  uint32_t protocolVersion = (0x5) << 28;
  uint32_t id = (0xACE) << 16;
  uint32_t n_words = (0x7) << 8;
  uint32_t type = 0x1 << 4;
  uint32_t info = 0xe;

  //printf("%02x\n", protocolVersion);
  //printf("%02x\n", id);
  //printf("%02x\n", n_words);

  uint32_t headerword = (protocolVersion | id | n_words | type | info);
  uint32_t expected = ipbus_transaction_header(5, 0xACE, 7, 1, 0xe);

  //printf("built manually: %02x\n", headerword);
  //printf("from ipbus.c: %02x\n", expected);
  mu_assert("error, test_ipbus_transaction_header", 
      headerword == expected);
  return 0;
}

static char* test_ipbus_packet_header(void) {
  uint32_t expected = 0x20FADEf2;
  mu_assert_eq("hdr", expected, ipbus_packet_header(0xFADE, 2));
  return 0;
}

static char* test_ipbus_detect_packet_header() {
  uint32_t aheader = ipbus_packet_header(0xBEEF, 0);
  mu_assert_eq("hdr", ipbus_detect_packet_header(aheader), IPBUS_ISTREAM_PACKET);
  mu_assert_eq("hdr swapped", ipbus_detect_packet_header(__bswap_32(aheader)), IPBUS_ISTREAM_PACKET_SWP_ORD);
  mu_assert_eq("not a hdr", ipbus_detect_packet_header(0xDEADBEEF), 0);
  mu_assert_eq("a transaction", ipbus_detect_packet_header(ipbus_transaction_header(2, 0xEEF, IPBUS_READ, 2, 0xf)), 0);
  return 0;
}

static char* test_ipbus_stream_state() {
  int swapbytes = 2; 
  CircularBuffer* test_buffer = cbuffer_new();
  cbuffer_push_back_net(test_buffer, ipbus_packet_header(0xBEEF, 2));
  cbuffer_push_back_net(test_buffer, 0xBADD); // garbage, shouldn't matter

  mu_assert_eq("hdr in stream", ipbus_stream_state(test_buffer, &swapbytes), IPBUS_ISTREAM_PACKET);
  mu_assert_eq("hdr endianness detect", swapbytes, 0);

  cbuffer_deletefront(test_buffer, 2);

  // A read request
  cbuffer_push_back_net(test_buffer, ipbus_transaction_header(2, 0xEEF, 2, IPBUS_READ, IPBUS_INFO_REQUEST));
  cbuffer_push_back_net(test_buffer, 0xDEADBEEF);
  // We expect one extra word (the base addr)
  mu_assert_eq("read length", ipbus_transaction_payload_size(2, IPBUS_READ, IPBUS_INFO_REQUEST), 1);
  mu_assert_eq("trns in stream", ipbus_stream_state(test_buffer, &swapbytes), IPBUS_ISTREAM_FULL_TRANS);

  cbuffer_deletefront(test_buffer, 2);

  // A write request of 8 words, that isn't fully buffered
  cbuffer_push_back_net(test_buffer, ipbus_transaction_header(2, 0xEEF, 8, IPBUS_WRITE, IPBUS_INFO_REQUEST));

  // a base addr + 8 data words
  mu_assert_eq("write length", ipbus_transaction_payload_size(8, IPBUS_WRITE, IPBUS_INFO_REQUEST), 9);
  mu_assert_eq("trns partial", ipbus_stream_state(test_buffer, &swapbytes), IPBUS_ISTREAM_PARTIAL_TRANS);

  cbuffer_deletefront(test_buffer, 1);

  mu_assert_eq("empty", ipbus_stream_state(test_buffer, &swapbytes), IPBUS_ISTREAM_EMPTY);

  return 0;
}

// Test decoding a transaction stream header-only
static char* test_ipbus_decode_transaction_header(void) {
  CircularBuffer* packet = cbuffer_new();
  // a read request.
  uint32_t header = ipbus_transaction_header(
      2, // protocol
      0xFEE, // transaction id
      5, // number of words to read
      IPBUS_READ,
      IPBUS_INFO_REQUEST);

  cbuffer_push_back_net(packet, header);

  ipbus_transaction_t decoded = ipbus_decode_transaction_header(packet, 0);

  mu_assert("trans decode err, id", decoded.id == 0xFEE);
  mu_assert("trans decode err, words", decoded.words == 5);
  mu_assert("trans decode err, info", decoded.info == IPBUS_INFO_REQUEST);
  mu_assert("trans decode err, type", decoded.type == IPBUS_READ);
  mu_assert("trans decode err, datasize", decoded.data.size == 1); // defined by IPBUS_READ
  // do nothing w/ the data
  mu_assert("trans decode err, data", decoded.data.words == NULL);

  mu_assert("trans size", ipbus_transaction_endocded_size(&decoded) == 2);
  return 0;
}

// Test decoding a transaction stream header-only
static char* test_ipbus_decode_write_transaction_header(void) {
  CircularBuffer* packet = cbuffer_new();
  // a read request.
  uint32_t header = ipbus_transaction_header(
      2, // protocol
      0xFEE, // transaction id
      5, // number of words to write
      IPBUS_WRITE,
      IPBUS_INFO_REQUEST);

  cbuffer_push_back_net(packet, header);

  ipbus_transaction_t decoded = ipbus_decode_transaction_header(packet, 0);

  mu_assert("trans decode err, id", decoded.id == 0xFEE);
  mu_assert("trans decode err, words", decoded.words == 5);
  mu_assert("trans decode err, info", decoded.info == IPBUS_INFO_REQUEST);
  mu_assert("trans decode err, type", decoded.type == IPBUS_WRITE);
  mu_assert("trans decode err, datasize", decoded.data.size == 6); // defined by IPBUS_WRITE + 5
  // do nothing w/ the data
  mu_assert("trans decode err, data", decoded.data.words == NULL);

  return 0;
}


// Test decoding a transaction stream
static char* test_ipbus_decode_transaction(void) {
  CircularBuffer* packet = cbuffer_new();
  // a read request.
  uint32_t header = ipbus_transaction_header(
      2, // protocol
      0xACE, // transaction id
      5, // number of words to read
      IPBUS_READ,
      IPBUS_INFO_REQUEST);
  uint32_t payload = 0xBEEFFACE;

  cbuffer_push_back_net(packet, header);
  cbuffer_push_back_net(packet, payload);

  ipbus_transaction_t decoded = ipbus_decode_transaction(packet, 0);

  //HEX_PRINT(packet[0]);
  //HEX_PRINT(packet[1]);
  //HEX_PRINT(decoded.id);

  //mu_assert("trans decode err, prot", decoded.protocol == 2);
  mu_assert("trans decode err, id", decoded.id == 0xACE);
  mu_assert("trans decode err, words", decoded.words == 5);
  mu_assert("trans decode err, info", decoded.info == IPBUS_INFO_REQUEST);
  mu_assert("trans decode err, type", decoded.type == IPBUS_READ);
  mu_assert("trans decode err, datasize", decoded.data.size == 1); // defined by IPBUS_READ
  mu_assert("trans decode err, data", decoded.data.words[0] == 0xBEEFFACE);

  mu_assert("trans size", ipbus_transaction_endocded_size(&decoded) == 2);
  return 0;
}

// Test packet size computatation
static char* test_ipbus_transaction_payload_size(void) {
  mu_assert("payload read req", ipbus_transaction_payload_size(5, IPBUS_READ, IPBUS_INFO_REQUEST)==1);
  mu_assert("payload read resp", ipbus_transaction_payload_size(5, IPBUS_READ, IPBUS_INFO_SUCCESS)==5);
  mu_assert("payload niread req", ipbus_transaction_payload_size(5, IPBUS_NIREAD, IPBUS_INFO_REQUEST)==1);
  mu_assert("payload niread resp", ipbus_transaction_payload_size(5, IPBUS_NIREAD, IPBUS_INFO_SUCCESS)==5);
  mu_assert("payload write req", ipbus_transaction_payload_size(5, IPBUS_WRITE, IPBUS_INFO_REQUEST)==6);
  mu_assert("payload write resp", ipbus_transaction_payload_size(5, IPBUS_WRITE, IPBUS_INFO_SUCCESS)==0);
  mu_assert("payload niwrite req", ipbus_transaction_payload_size(5, IPBUS_NIWRITE, IPBUS_INFO_REQUEST)==6);
  mu_assert("payload niwrite resp", ipbus_transaction_payload_size(5, IPBUS_NIWRITE, IPBUS_INFO_SUCCESS)==0);
  // num words is ignored
  mu_assert("payload rwm req", ipbus_transaction_payload_size(9, IPBUS_RMW, IPBUS_INFO_REQUEST)==3);
  mu_assert("payload rwm resp", ipbus_transaction_payload_size(7, IPBUS_RMW, IPBUS_INFO_SUCCESS)==1);
  mu_assert("payload rwm req", ipbus_transaction_payload_size(9, IPBUS_RMWSUM, IPBUS_INFO_REQUEST)==2);
  mu_assert("payload rwm resp", ipbus_transaction_payload_size(7, IPBUS_RMWSUM, IPBUS_INFO_SUCCESS)==1);
  return 0;
}

// Test decoding + re-encoding a transaction stream
static char* test_ipbus_decode_encode_transaction(void) {
  CircularBuffer* packet = cbuffer_new();
  // a read request.
  uint32_t header = ipbus_transaction_header(
      2, // protocol
      0xACE, // transaction id
      5, // number of words to read
      IPBUS_READ,
      IPBUS_INFO_REQUEST);
  uint32_t payload = 0xBEEFFACE;

  cbuffer_push_back_net(packet, header);
  cbuffer_push_back_net(packet, payload);

  ipbus_transaction_t decoded = ipbus_decode_transaction(packet, 0);

  CircularBuffer* encoded = cbuffer_new();
  ipbus_encode_transaction(encoded, &decoded, 0);

  mu_assert_eq("encode-decode length", cbuffer_size(encoded), 2);

  mu_assert("encode-decode", memcmp(encoded->data, packet->data, 8)==0);

  return 0;
}


static char* test_ipbus_decode_encode_write_transaction(void) {
  CircularBuffer* packet = cbuffer_new();
  // a read request.
  uint32_t header = ipbus_transaction_header(
      2, // protocol
      0xACE, // transaction id
      5, // number of words to write
      IPBUS_WRITE,
      IPBUS_INFO_REQUEST);
  uint32_t baseaddr = 0xBEEFFACE;

  cbuffer_push_back_net(packet, header);
  cbuffer_push_back_net(packet, baseaddr);
  for (size_t i = 0; i < 5; ++i) {
    cbuffer_push_back_net(packet, i);
  }

  ipbus_transaction_t decoded = ipbus_decode_transaction(packet, 0);

  CircularBuffer* encoded = cbuffer_new();
  ipbus_encode_transaction(encoded, &decoded, 0);

  mu_assert_eq("encode-decode length", cbuffer_size(encoded), 1 + 1 + 1 * 5);

  mu_assert("encode-decode", memcmp(encoded->data, packet->data, 7*4)==0);

  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== test_serialization ===\n");
  mu_run_test(test_ipbus_transaction_header);
  mu_run_test(test_ipbus_packet_header);
  mu_run_test(test_ipbus_stream_state);
  mu_run_test(test_ipbus_detect_packet_header);
  mu_run_test(test_ipbus_decode_transaction_header);
  mu_run_test(test_ipbus_decode_write_transaction_header);
  mu_run_test(test_ipbus_decode_transaction);
  mu_run_test(test_ipbus_transaction_payload_size);
  mu_run_test(test_ipbus_decode_encode_transaction);
  mu_run_test(test_ipbus_decode_encode_write_transaction);
  return 0;
}
