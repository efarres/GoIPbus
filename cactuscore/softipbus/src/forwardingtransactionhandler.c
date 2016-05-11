/*
 * =====================================================================================
 *
 *       Filename:  forwardingtransactionhandler.cc
 *
 *    Description:  Forward a transaction stream along a serial bus for further processing.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include "transactionhandler.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

#include "circular_buffer.h"
#include "serialization.h"
#include "macrologger.h"
#include "tty_functions.h"

static int forwarding_tx_fd = -1;
static int forwarding_rx_fd = -1;
static fd_set forwarding_fdset;
static int maxfd;

#ifndef IPBUS_FORWARD_TX
#define IPBUS_FORWARD_TX /fix/me
#endif
#ifndef IPBUS_FORWARD_RX
#define IPBUS_FORWARD_RX /fix/me
#endif

#define xstringify(s) stringify(s)
#define stringify(s) #s

void initialize_fowarding_fds(int txfd, int rxfd) {
  if (forwarding_tx_fd != -1) {
    LOG_ERROR("Fowarding file descriptors already set up! wtf");
  }
  // set the file descriptors manually
  if (txfd != 0 || rxfd != 0) {
    forwarding_tx_fd = txfd;
    forwarding_rx_fd = rxfd;
  } else {
    char * tx = xstringify(IPBUS_FORWARD_TX);
    char * rx = xstringify(IPBUS_FORWARD_RX);
    LOG_INFO("Forwarding transations with TX: %s and RX: %s", tx, rx);
    forwarding_tx_fd = open(tx, O_RDWR | O_NOCTTY | O_SYNC);
    ttySetRaw(forwarding_tx_fd, NULL);
    if (strcmp(tx, rx) != 0) {
      forwarding_rx_fd = open(rx, O_RDWR | O_NOCTTY | O_SYNC);
      ttySetRaw(forwarding_rx_fd, NULL);
    } else {
      forwarding_rx_fd = forwarding_tx_fd;
    }
  }
  FD_ZERO(&forwarding_fdset);
  FD_SET(forwarding_rx_fd, &forwarding_fdset);
  if (forwarding_rx_fd != forwarding_tx_fd) {
    FD_SET(forwarding_tx_fd, &forwarding_fdset);
  }
  maxfd = forwarding_tx_fd > forwarding_rx_fd ? forwarding_tx_fd : forwarding_rx_fd;
}

int handle_transaction_stream(CircularBuffer* input, int swapbytes, CircularBuffer* output) {
  // Check if the FDs have not already been setup manually.
  if (forwarding_tx_fd == -1) {
    initialize_fowarding_fds(0, 0);
  }
  // Read as many *full* transaction packets as are in the input buffer.
  size_t expected_response_words = 0;
  size_t transactions_read_size = 0;
  size_t response_received_words = 0;
  int dummy = 0;
  while (ipbus_stream_state(input, &dummy) == IPBUS_ISTREAM_FULL_TRANS) {
    ipbus_transaction_t input_skeleton = ipbus_decode_transaction_header(input, swapbytes);
    size_t this_transaction_size = ipbus_transaction_endocded_size(&input_skeleton);
    transactions_read_size += this_transaction_size;
    // write the packet to the fd - input will be consumed.
    // Note: once we are in fd-forwarding land, we always assume native
    // endianness.
    LOG_DEBUG("Forwarding 1 transaction"); 
    size_t need_to_send = this_transaction_size;
    while (need_to_send) {
      // Poll the incoming serial port if we need to read out any RX bits
      fd_set readfds;
      memcpy(&readfds, &forwarding_fdset, sizeof(fd_set));
      fd_set writefds;
      memcpy(&writefds, &forwarding_fdset, sizeof(fd_set));
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      select(maxfd+1, &readfds, NULL, NULL, &timeout);

      if (FD_ISSET(forwarding_rx_fd, &readfds)) {
        cbuffer_read_fd(output, forwarding_rx_fd, 1);
        response_received_words++;
      }

      if (FD_ISSET(forwarding_tx_fd, &writefds)) {
        cbuffer_write_fd(input, forwarding_tx_fd, 1);
        need_to_send -= 1;
      }

    }
    // Now determine the size of the expected response.  We have to wait her for
    // it to be done.  Maybe we can improve this in the future to improve latency.
    expected_response_words += 1; // the header
    switch (input_skeleton.type) {
      case IPBUS_READ:
      case IPBUS_NIREAD:
        expected_response_words += input_skeleton.words;
        break;
      case IPBUS_WRITE:
      case IPBUS_NIWRITE:
        // no response payload
        break;
      case IPBUS_RMW:
      case IPBUS_RMWSUM:
        expected_response_words += 1;
        break;
    }
  }
  LOG_DEBUG("Waiting for result"); 
  // Now wait for the response, and put it in the output buffer
  while (response_received_words < expected_response_words) {
    cbuffer_read_fd(output, forwarding_rx_fd, 1);
    response_received_words++;
  }
  LOG_DEBUG("Read back transactions"); 
  return transactions_read_size;
}
