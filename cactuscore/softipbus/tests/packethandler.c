/*
 * =====================================================================================
 *
 *       Filename:  test_packethandler.c
 *
 *    Description:  Tests of packet handling logic.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "minunit.h"

#include "endiantools.h"
#include "protocol.h"
#include "packethandler.h"
#include "serialization.h"

// build a fake packet with a read and RMW transaction.  If this is modified,
// make sure you also change test_ipbus_process_full_pkt, as it tests the reply.
CircularBuffer* build_fake_packet(void) {
  // packet ID = protocol 2, 0x0, id beef, 0xf, packet type 0.
  uint32_t packet_header = 0x20BEEFF0;
  CircularBuffer* output = cbuffer_new();
  cbuffer_push_back_net(output, packet_header);
  // now add a read packet - reading 5 words at 0xBEEFCAFE
  cbuffer_push_back_net(output, ipbus_transaction_header(2, 0xBAD, 5, IPBUS_READ, IPBUS_INFO_REQUEST));   
  cbuffer_push_back_net(output, 0xBEEFCAFE);
  // now a RMW packet, @0xBEEFCAFE, AND 0xDEADBEEF and OR 0xFACEBEEF
  cbuffer_push_back_net(output, ipbus_transaction_header(2, 0xCAB, 1, IPBUS_RMW, IPBUS_INFO_REQUEST));   
  cbuffer_push_back_net(output, 0xBEEFCAFE);
  cbuffer_push_back_net(output, 0xDEAFBEEF);
  cbuffer_push_back_net(output, 0xFACEBEEF);

  return output;
}

static char* test_ipbus_process_input_stream_empty_pkt(void) {
  Client client;
  client.inputstream = cbuffer_new();
  client.outputstream = cbuffer_new();

  mu_assert_eq("bufsize zero", cbuffer_size(client.inputstream), 0);

  // nothing should happen.
  ipbus_process_input_stream(&client);

  mu_assert_eq("output bufsize zero", cbuffer_size(client.outputstream), 0);

  return 0;
}

static char* test_ipbus_process_input_stream_hdr_pkt(void) {
  Client client;
  client.inputstream = cbuffer_new();
  client.outputstream = cbuffer_new();

  // put a fake packet header
  cbuffer_push_back_net(client.inputstream, 0x20BEEFF0);
  mu_assert_eq("bufsize nonzero", cbuffer_size(client.inputstream), 1);

  ipbus_process_input_stream(&client);

  // this should have put a reply packet header on the wire
  mu_assert_eq("words read", cbuffer_size(client.outputstream), 1);
  // exact same format
  mu_assert_eq("reply", 0x20BEEFF0, cbuffer_value_at_net(client.outputstream, 0));

  // and consumed the packet.
  mu_assert_eq("packet nommed", cbuffer_size(client.inputstream), 0);

  return 0;
}

static char* test_ipbus_process_input_stream_hdr_pkt_swapped(void) {
  Client client;
  client.inputstream = cbuffer_new();
  client.outputstream = cbuffer_new();
  client.swapbytes = 0;

  // put a fake packet header with swapped endianness
  cbuffer_push_back_net(client.inputstream, __bswap_32(0x20BEEFF0));

  ipbus_process_input_stream(&client);

  // swapbytes should now be set
  mu_assert_eq("swapbytes set", client.swapbytes, 1);

  mu_assert_eq("bytes read", cbuffer_size(client.outputstream), 1);
  // exact same format as input, respecting endianness
  mu_assert_eq("reply", __bswap_32(0x20BEEFF0), 
      cbuffer_value_at_net(client.outputstream, 0));

  // and consumed the packet.
  mu_assert_eq("packet nommed", cbuffer_size(client.inputstream), 0);

  return 0;
}

static char* test_ipbus_process_full_pkt_consumption(void) {
  Client client;
  client.inputstream = build_fake_packet();
  client.outputstream = cbuffer_new();
  client.swapbytes = 0;

  int words_in_buffer = cbuffer_size(client.inputstream);
  int words_processed = ipbus_process_input_stream(&client);

  mu_assert_eq("swapbytes set", client.swapbytes, 0);

  mu_assert_eq("ate everything", words_processed, words_in_buffer);
  // and consumed the packet.
  mu_assert_eq("packet nommed", cbuffer_size(client.inputstream), 0);

  return 0;
}

static char* test_ipbus_process_full_pkt_reply(void) {
  Client client;
  client.inputstream = build_fake_packet();
  client.outputstream = cbuffer_new();
  client.swapbytes = 0;

  cbuffer_size(client.inputstream);
  ipbus_process_input_stream(&client);

  // This should have put a reply packet on the wire.  Let's parse it.
  CircularBuffer* mybuf = client.outputstream;
  int words_expected = (
    1 // packet header
    + 1 // read 5 words reply header
    + 5 // read 5 words payload
    + 1 // RMW reply header
    + 1 // RMW before-modify value
    );

  mu_assert_eq("words written", cbuffer_size(mybuf), words_expected);
  mu_assert_eq("header", 0x20BEEFF0, cbuffer_value_at_net(mybuf, 0));
  mu_assert_eq("read header", 0x2BAD0500, cbuffer_value_at_net(mybuf, 1));
  for (int i = 0; i < 5; ++i) {
    mu_assert_eq("read payload", i+1, cbuffer_value_at_net(mybuf, i + 2));
  }
  mu_assert_eq("rmw header", 0x2CAB0140, cbuffer_value_at_net(mybuf, 7));
  mu_assert_eq("rmw header", 
   (0xBEEFCAFE & 0xDEAFBEEF) | 0xFACEBEEF, cbuffer_value_at_net(mybuf, 8));
  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== test_packethandler ===\n");
  mu_run_test(test_ipbus_process_input_stream_hdr_pkt);
  mu_run_test(test_ipbus_process_input_stream_hdr_pkt_swapped);
  mu_run_test(test_ipbus_process_input_stream_empty_pkt);
  mu_run_test(test_ipbus_process_full_pkt_consumption);
  mu_run_test(test_ipbus_process_full_pkt_reply);
  return 0;
}
