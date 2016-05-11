/*
 * =====================================================================================
 *
 *       Filename:  handlers.c
 *
 *    Description:  Implementations of IPBus memory peeker and poker
 *                  transactions.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macrologger.h"
#include "membase.h"
#include "handlers.h"

#include <inttypes.h>

// Read functions return a buffer of data
ipbus_payload_t handle_IPBUS_READ(uint8_t nwords, uint32_t base_address) {
  LOG_DEBUG("==> IPBUS_READ     nwords: %"PRIx8" @ addr: %"PRIx32, nwords, base_address);
  ipbus_payload_t output;
  output.size = nwords;
  output.words = (uint32_t*)malloc(nwords * sizeof(uint32_t));
  memcpy(output.words, membase + base_address, nwords * sizeof(uint32_t));
  /*  
  for (int i = 0; i < output.size; ++i) {
    LOG_DEBUG("Reading %016"PRIxPTR" => %08x", (uintptr_t)(membase + base_address  * sizeof(uint32_t) + i * sizeof(uint32_t)), output.words[i]);
  }
  */
  return output;
}

ipbus_payload_t handle_IPBUS_NIREAD(uint8_t nwords, uint32_t base_address) {
  LOG_DEBUG("==> IPBUS_NIREAD   nwords: %"PRIx8" @ addr: %"PRIx32, nwords, base_address);
  ipbus_payload_t output;
  output.size = nwords;
  output.words = (uint32_t*)malloc(nwords * sizeof(uint32_t));
  // read the same place a bunch of times. think we need to fix this?
  for (int i = 0; i < nwords; ++i) {
    output.words[i] = *((volatile uint32_t*) (membase + base_address));
  }
  return output;
}

// Write functions return no data - return value should indicate whether
// write was successful.  The write address is the first word of the data.
int handle_IPBUS_WRITE(uint8_t writesize, const ipbus_payload_t* data) {
  LOG_DEBUG("==> IPBUS_WRITE    writesize: %"PRIx8" @ addr: %"PRIx32, writesize, data->words[0]);
  memcpy(membase + data->words[0], data->words + 1, writesize * sizeof(uint32_t));
  /*  
  for (int i = 1; i < data->size; ++i) {
    LOG_DEBUG("writing %016"PRIxPTR" <= %08x", (uintptr_t)(membase + data->words[0] * sizeof(uint32_t) + (i - 1)*sizeof(uint32_t)), data->words[i]);
  }
  */
  return 0;
}

int handle_IPBUS_NIWRITE(uint8_t writesize, const ipbus_payload_t* data) {
  LOG_DEBUG("==> IPBUS_NIWRITE  writesize: %"PRIx8" @ addr: %"PRIx32, writesize, data->words[0]);
  // write the same place a bunch of times. think we need to fix this?
  for (int i = 1; i < data->size; ++i) {
    *(uint32_t*)(membase + data->words[0]) = data->words[i];
  }
  return 0;
}

// Read write modifies returns original contents at address
uint32_t handle_IPBUS_RMW(uint32_t base_address, uint32_t andterm, uint32_t orterm) {
  LOG_DEBUG("==> IPBUS_RMW      @ addr: %"PRIx32" - AND: %"PRIx32" OR: %"PRIx32, 
      base_address, andterm, orterm);
  uint32_t current = *(uint32_t*)(membase + base_address);
  *(uint32_t*)(membase + base_address) = (current & andterm) | orterm;
  return current;
}

uint32_t handle_IPBUS_RMWSUM(uint32_t base_address, uint32_t addend) {
  LOG_DEBUG("==> IPBUS_RMWSUM   @ addr: %"PRIx32" - +: %"PRIx32, 
      base_address, addend);
  uint32_t current = *(uint32_t*)(membase + base_address);
  *(uint32_t*)(membase + base_address) = current + addend;
  return current;
}
