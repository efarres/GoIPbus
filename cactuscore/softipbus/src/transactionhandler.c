/*
 * The IPBus transaction handler.
 *
 * This function reads a byte stream, decodes it into an ipbus_transaction_t
 * and then dispatches the required action via the ipbus_process_transaction
 * function.
 *
 * Author: Evan K. Friis, UW Madison
 *
 */
#include "transactionhandler.h"

#include <stdlib.h>

#include "macrologger.h"
#include "serialization.h"
#include "handlers.h"

static ipbus_transaction_t ipbus_process_transaction(const ipbus_transaction_t* input) {
  ipbus_transaction_t output;
  output.id = input->id;
  output.type = input->type;
  output.info = IPBUS_INFO_SUCCESS;
  output.words = input->words;
  output.data.words = NULL;
  output.data.size = 0;

  switch (output.type) {
    case IPBUS_READ:
      output.data = handle_IPBUS_READ(input->words, input->data.words[0]);
      break;
    case IPBUS_NIREAD:
      output.data = handle_IPBUS_NIREAD(input->words, input->data.words[0]);
      break;
    case IPBUS_WRITE:
      handle_IPBUS_WRITE(input->words, &(input->data));
      break;
    case IPBUS_NIWRITE:
      handle_IPBUS_NIWRITE(input->words, &(input->data));
      break;
    case IPBUS_RMW:
      output.data.size = 1;
      output.data.words = (uint32_t*)malloc(sizeof(uint32_t));
      output.data.words[0] = handle_IPBUS_RMW(
          input->data.words[0], input->data.words[1], input->data.words[2]);
      break;
    case IPBUS_RMWSUM:
      output.data.size = 1;
      output.data.words = (uint32_t*)malloc(sizeof(uint32_t));
      output.data.words[0] = handle_IPBUS_RMWSUM(
          input->data.words[0], input->data.words[1]);
      break;
  }

  return output;
}

int handle_transaction_stream(CircularBuffer* input, int swapbytes, CircularBuffer* output) {
  ipbus_transaction_t trans_req = ipbus_decode_transaction(input, swapbytes);
  LOG_DEBUG("Processing transaction %03x", trans_req.id);
  // indicate how much we consumed so we can delete it from the input stream
  int wordsprocessed = ipbus_transaction_endocded_size(&trans_req);
  cbuffer_deletefront(input, wordsprocessed);
  // perform request action(s) and prepare response
  ipbus_transaction_t trans_resp = ipbus_process_transaction(&trans_req);
  ipbus_encode_transaction(output, &trans_resp, swapbytes);
  // cleanup input and output transactions memory
  free(trans_resp.data.words);
  free(trans_req.data.words);
  return wordsprocessed;
}

