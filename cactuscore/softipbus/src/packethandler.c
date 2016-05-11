#include "packethandler.h"

#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include "endiantools.h"
#include "macrologger.h"
#include "protocol.h"
#include "serialization.h"
#include "transactionhandler.h"

size_t ipbus_process_input_stream(Client* client) {
  // check the state of the stream, and update the clients endian-ness (maybe)
  int stream_state = ipbus_stream_state(client->inputstream, &(client->swapbytes));
  size_t data_processed = 0;
  switch (stream_state) {

    case IPBUS_ISTREAM_FULL_TRANS: 
      { 
        LOG_DEBUG("Processing full transaction");
        // why the brace: http://stackoverflow.com/questions/1231198/declaring-variables-inside-a-switch-statement
        data_processed += handle_transaction_stream(client->inputstream, client->swapbytes, client->outputstream);
        // handle_transaction_stream will delete any handled data from 
        // the input stream.
        break; 
      }

    case IPBUS_ISTREAM_PACKET:
    case IPBUS_ISTREAM_PACKET_SWP_ORD:
      {
        uint32_t headerword = cbuffer_value_at_net(client->inputstream, 0);
        // now pop the data we processed off the input buffer
        cbuffer_deletefront(client->inputstream, 1);
        LOG_DEBUG("Got new header packet %"PRIx32, headerword);
        // by definition this is in the correct endianness for the client
        cbuffer_push_back_net(client->outputstream, headerword);
        // pop off the data we've just processed
        data_processed += 1;
        break;
      }

    case IPBUS_ISTREAM_EMPTY:
    case IPBUS_ISTREAM_PARTIAL_TRANS:
        LOG_DEBUG("Partial transaction");
        return 0; // and wait for more data
  }
  // recurse to process more data, if there is any.
  return data_processed + ipbus_process_input_stream(client);
}
