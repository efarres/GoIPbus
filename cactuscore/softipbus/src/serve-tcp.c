/*
 * =====================================================================================
 *
 *       Filename:  serve.c
 *
 *    Description:  Processes incoming TCP requests and sends them to the IPBus 
 *                  processor.
 *
 *         Author:  Jes Tikalsky, Evan Friis (UW Madison)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include "macrologger.h"
#include "membase.h"
#include "packethandler.h"
#include "client.h"

#define MEMORY_SIZE 1024*1024
#define MAX_CLIENTS 50

#ifndef PORT
#define PORT 60002
#endif

#define MAX_REQ_LEN 1472
/*  from the IPbus protocol 

    "It should be noted that in this example, each transaction or set of
    transactions must fit into a single UDP packet. Note that the maximum size of a
    standard Ethernet packet (without using jumbo frames) is 1500 bytes; with an IP
    header of 20 bytes and a UDP header of 8 bytes, this gives the maximum IPbus
    packet size of 368 32-bit words, or 1472 bytes. Longer block transfers must be
    split at the software level into individual packets."

*/


// These need to be global so we can close TCP connections on Ctrl-C
static fd_set fds;
static Client clients[MAX_CLIENTS]; 
static int numclients;

// Helper functions to close TCP connections
void disconnect_client(int client);
void disconnect_all_clients(void);
// Signal handler to shut down gracefully on Ctrl-C
void sig_handler(int signo);

int main(int argc, char *argv[]) {
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    LOG_ERROR("Can't catch SIGINT");
  }
  if (signal(SIGTERM, sig_handler) == SIG_ERR) {
    LOG_ERROR("Can't catch SIGTERM");
  }
  int listenfd;
  struct sockaddr_in serv_addr;

  // initialize clients
  numclients = 0;
  memset(clients, 0, sizeof(clients));

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    LOG_ERROR("Unable to open socket: %d (%s)", errno, strerror(errno));
    return 1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  serv_addr.sin_port = htons(PORT);

  int optval = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(int));

  if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    LOG_ERROR("Unable to bind address: %d (%s)", errno, strerror(errno));
    return 1;
  }

  listen(listenfd,5);

  fd_set readfds;
  FD_ZERO(&fds);
  FD_SET(listenfd, &fds);
  int maxfd = listenfd+1;

  // initialize mapped memory block
  membase_init();

  LOG_INFO("ipbus2mem serving memory @ %016" PRIxPTR " on port %i", 
      (uintptr_t)membase, PORT);

  while (1) {
    memcpy(&readfds, &fds, sizeof(fd_set));

    if (select(maxfd, &readfds, NULL, NULL, NULL) < 1)
      continue;

    if (FD_ISSET(listenfd, &readfds)) {
      int clientfd = accept(listenfd, NULL, NULL);
      if (clientfd < 0) {
        LOG_ERROR("Accept Error: %d (%s)", errno, strerror(errno));
      }
      else if (numclients >= MAX_CLIENTS) {
        close(clientfd);
      }
      else {
        LOG_INFO("Connecting client #%i", numclients);
        // sockets are bidirectional
        clients[numclients].inputfd = clientfd;
        clients[numclients].inputstream = cbuffer_new();
        clients[numclients].outputstream = cbuffer_new();
        clients[numclients].byte2word = bytebuffer_ctor(NULL, 0);
        numclients++;
        FD_SET(clientfd, &fds);
        if (clientfd >= maxfd)
          maxfd = clientfd + 1;
      }
    }

    for (int client = 0; client < numclients; client++) {
      int disconnect = 0;
      if (FD_ISSET(clients[client].inputfd, &readfds)) {
        LOG_DEBUG("Detected data from client #%i", client);
        int nbytes = bytebuffer_read_fd(
            &(clients[client].byte2word), clients[client].inputfd, MAX_REQ_LEN);

        if (nbytes == 0) {
          // EOF
          disconnect = 1;
        }
        if (clients[client].byte2word.bufsize) {
          LOG_DEBUG("Processing %i bytes of data from client #%i", 
              clients[client].byte2word.bufsize, client);
          // Data available!  Process the request.
          CircularBuffer *clibuf = clients[client].inputstream;
          ByteBuffer* byte2word = &(clients[client].byte2word);
          size_t nwords = byte2word->bufsize / sizeof(uint32_t);
          int append_status = cbuffer_append(clibuf, byte2word->buf, nwords);
          if (append_status == 0) {
            bytebuffer_del_front(byte2word, nwords * sizeof(uint32_t));
          }
          //int rv = 
          LOG_DEBUG("Processing %i words of data", cbuffer_size(clibuf));
          ipbus_process_input_stream(&clients[client]);
          // right now this returns bytes processed...
          //disconnect = (rv == 1);
          // write out any response
          cbuffer_write_fd(clients[client].outputstream,
              clients[client].inputfd,
              cbuffer_size(clients[client].outputstream));
        }
      }
      if (disconnect) {
        disconnect_client(client);
        client--;
      }
    }
  }
  // Make sure all conenctions are cleaned up.
  disconnect_all_clients();
  close(listenfd);
  membase_close();
  LOG_INFO("goodbye!\n");
  return 0; 
}

void disconnect_client(int client) {
  FD_CLR(clients[client].inputfd, &fds);
  close(clients[client].inputfd);

  cbuffer_free(clients[client].inputstream);
  cbuffer_free(clients[client].outputstream);
  bytebuffer_free(&(clients[client].byte2word));

  for (int i = client; i < numclients-1; i++)
    clients[i] = clients[i+1];
  numclients--;
  LOG_INFO("Disconnected client #%i", client);
}

void disconnect_all_clients(void) {
  while (numclients) {
    disconnect_client(numclients - 1);
  }
}

void sig_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    LOG_INFO("shutting down");
    disconnect_all_clients();
    exit(0);
  }
}

