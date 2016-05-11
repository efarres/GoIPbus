/*
 * =====================================================================================
 *
 *       Filename:  transactionhandler.h
 *
 *    Description:  Handles an input stream which contains (at least) one full
 *                  transaction.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#ifndef IBPUS_TRANSACTIONHANDLER_H
#define IPBUS_TRANSACTIONHANDLER_H

#include "circular_buffer.h"

// Handle a stream of data from <input>.  Appends the encoded transaction
// response to <output>.  Assumes that input buffer contains a *FULL
// TRANSACTION*.  Returns number of words processed, which are consumed from
// the input.
int handle_transaction_stream(CircularBuffer* input, int swapbytes,
    CircularBuffer* output);

#endif
