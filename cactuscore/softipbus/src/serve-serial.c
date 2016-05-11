/*
 * =====================================================================================
 *
 *       Filename:  serve-serial.c
 *
 *    Description:  Receive IPBus transactions over file descriptors.  This is
 *                  mainly intending for testing the forwarding functionality.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>

#include "macrologger.h"
#include "membase.h"
#include "packethandler.h"
#include "client.h"

#define MEMORY_SIZE 1024*1024
#define MAX_CLIENTS 50
#define PORT 60002


#define MAX_REQ_LEN 1472
/*  from the IPbus protocol 

    "It should be noted that in this example, each transaction or set of
    transactions must fit into a single UDP packet. Note that the maximum size
    of a standard Ethernet packet (without using jumbo frames) is 1500 bytes;
    with an IP header of 20 bytes and a UDP header of 8 bytes, this gives the
    maximum IPbus packet size of 368 32-bit words, or 1472 bytes. Longer block
    transfers must be split at the software level into individual packets."

*/


static int caught_termination;

void sig_handler(int signo)
{
  if (signo == SIGINT || signo == SIGTERM) {
    LOG_INFO("shutting down");
    caught_termination = 1;
  }
}

int main(int argc, char *argv[]) {
  caught_termination = 0;
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    LOG_ERROR("Can't catch SIGINT");
  }
  if (signal(SIGTERM, sig_handler) == SIG_ERR) {
    LOG_ERROR("Can't catch SIGTERM");
  }
  if (argc < 3) {
    LOG_ERROR("Usage: %s /dev/input /dev/output", argv[0]);
    return 1;
  }
  char * inputdevice = argv[1];
  char * outputdevice = argv[2];

  // initialize mapped memory block
  membase_init();

  LOG_INFO("serving memory @ %016" PRIxPTR " via %s-> ->%s", 
      (uintptr_t)membase, inputdevice, outputdevice);

  int inputdevicefd = open(inputdevice, O_RDWR | O_NONBLOCK);
  // in general these are probably the same.
  int outputdevicefd = inputdevicefd;
  // but maybe not.
  if (strcmp(inputdevice, outputdevice) != 0) {
    outputdevicefd = open(outputdevice, O_RDWR | O_NONBLOCK);
  }

  Client client;
  client.inputstream = cbuffer_new();
  client.outputstream = cbuffer_new();
  client.inputfd = inputdevicefd;
  client.outputfd = outputdevicefd;
  client.byte2word = bytebuffer_ctor(NULL, 0);
  client.swapbytes = 0;

  while (1) {
    if (caught_termination) {
      break;
    }

    bytebuffer_read_fd(
        &(client.byte2word), client.inputfd, MAX_REQ_LEN);
    if (client.byte2word.bufsize > 0) {
      LOG_DEBUG("Processing %i bytes", client.byte2word.bufsize);
      // Data available!  Process the request.
      CircularBuffer *clibuf = client.inputstream;
      ByteBuffer* byte2word = &(client.byte2word);
      size_t nwords = byte2word->bufsize / sizeof(uint32_t);
      int append_ret = cbuffer_append(clibuf, byte2word->buf, nwords);
      if (append_ret == 0) {
        bytebuffer_del_front(byte2word, nwords * sizeof(uint32_t));
      }
      ipbus_process_input_stream(&client);
      // write out any response
      cbuffer_write_fd(client.outputstream,
          client.outputfd,
          cbuffer_size(client.outputstream));
    }
  }

  if (inputdevicefd != outputdevicefd) {
    close(outputdevicefd);
  }
  close(inputdevicefd);
  membase_close();
  LOG_INFO("goodbye!\n");
  return 0; 
}
