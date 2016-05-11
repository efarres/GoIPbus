/*
 * =====================================================================================
 *
 *       Filename:  test_packethandler.c
 *
 *    Description:  Forwarding transactions along a file descriptor.
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

#include "protocol.h"
#include "transactionhandler.h"
#include "serialization.h"

void initialize_fowarding_fds(int tx, int rx);

static char * test_forwardingtransaction_handle_one_transaction(void) {
  // make two pipes to simulate the forwarding serial port.
  int txpipefd[2];
  pipe(txpipefd);

  int rxpipefd[2];
  pipe(rxpipefd);

  // make txpipe nonblocking, so we can check if it's empty.
  fcntl(txpipefd[0], F_SETFL, fcntl(txpipefd[0], F_GETFL) | O_NONBLOCK);

  initialize_fowarding_fds(txpipefd[1], rxpipefd[0]);

  CircularBuffer* input = cbuffer_new();
  cbuffer_push_back_net(input, ipbus_transaction_header(2, 0xCAB, 1, IPBUS_RMW, IPBUS_INFO_REQUEST));
  cbuffer_push_back_net(input, 0xBEEFCAFE);
  cbuffer_push_back_net(input, 0xDEAFBEEF);
  cbuffer_push_back_net(input, 0xFACEBEEF);
  // RMW expects 1+1 words back
  cbuffer_push_back_net(input, ipbus_transaction_header(2, 0xBAD, 5, IPBUS_READ, IPBUS_INFO_REQUEST));
  cbuffer_push_back_net(input, 0xBEEFCAFE);
  // READ expects 1+5 words back
  
  // write some junk onto the receiving pipe so we can check
  // we are reading the correct amount of return bytes.
  uint32_t junkwords[8] = {
    0xDEADBEEF, 0xBEEFCAFE, 
    0xFACEBEEF, 0x12345678, 0xDEADFACE, 0xBADEBEEF, 0x87654321, 0xABABABAB};
  write(rxpipefd[1], junkwords, 8 * sizeof(uint32_t));

  CircularBuffer* input_copy = cbuffer_copy(input);

  CircularBuffer* output = cbuffer_new();
  int words_consumed = handle_transaction_stream(input, 0, output);
  // should eat both transactions
  mu_assert_eq("ate everything i should", words_consumed, 6);

  // Make sure it passed it along the TX pipe
  ByteBuffer outputbuf = bytebuffer_ctor(NULL, 10 * sizeof(uint32_t));
  // should pass along only 6 words.
  read(txpipefd[0], outputbuf.buf, 10 * sizeof(uint32_t));
  mu_assert("forwarded trans", memcmp(outputbuf.buf, input_copy->data, 6*sizeof(uint32_t)) == 0);
  // there should no more data on the pipe
  mu_assert_eq("output pipe empty", read(txpipefd[0], outputbuf.buf, sizeof(uint32_t)), -1);

  // we expect header word + 1 payload word to be read from a RMW, 
  // 1 header + 5 data words from the READ
  mu_assert_eq("output size", cbuffer_size(output), 8);
  mu_assert_eq("output content header",  cbuffer_value_at(output, 0), 0xDEADBEEF);
  mu_assert_eq("output content payload", cbuffer_value_at(output, 1), 0xBEEFCAFE);
  mu_assert_eq("output content header 2", cbuffer_value_at(output, 2), 0xFACEBEEF);
  mu_assert_eq("output content payload 2", cbuffer_value_at(output, 3), 0x12345678);

  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== test_forwardingtransactionhandler ===\n");
  mu_run_test(test_forwardingtransaction_handle_one_transaction);
  return 0;
}
