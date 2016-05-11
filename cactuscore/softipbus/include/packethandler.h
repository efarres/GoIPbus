/*
 * =====================================================================================
 *
 *       Filename:  packethandler.h
 *
 *    Description:  Parses, executes transactions, and replies to IPbus packets.
 *
 *                  Since we don't worry about the reliability mechanism, we
 *                  always use IPbus packet ID = 0.  An IPBus packet header is
 *                  replied to immediately, so the system should always be in a
 *                  'mid-ipbus-packet' state.
 *
 *                  It then looks at the first word in the buffer.  If the
 *                  buffer is empty or has less than a word in it
 *                  (ISTREAM_EMPTY), it returns and waits for more data.  If the
 *                  first word is a packet (ISTREAM_PKT), it is popped and
 *                  immediately replied to.
 *
 *                  If it is a transaction, it checks the expected transaction
 *                  size.  If the buffer contains the whole transaction,
 *                  (ISTREAM_FULL_TRANS) it is parsed and responded to.  If the
 *                  transaction isn't fully loaded, it returns and waits for
 *                  more data.
 *
 *         Author:  Jes Tikalsky, Evan Friis (UW Madison)
 *
 * =====================================================================================
 */

#ifndef IPBUS_PACKETHANDLER_H
#define IPBUS_PACKETHANDLER_H

#include <stdint.h>

#include "client.h"
#include "protocol.h"

// Process a TCP/IP packet with content stored in buffer coming from the given
// client. The buffer in the client will be consumed.  Returns number of bytes
// processed.  The response will be written to the client's output buffer.
size_t ipbus_process_input_stream(Client* client);

#endif
