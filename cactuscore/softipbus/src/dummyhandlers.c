/*
 * =====================================================================================
 *
 *       Filename:  logging_handlers.c
 *
 *    Description:  IPbus handler functions which return dummy data, logging actions
 *                  to stdout.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <stdlib.h>

#include "handlers.h"
#include "macrologger.h"

// Read functions return a buffer of data
ipbus_payload_t handle_IPBUS_READ(uint8_t nwords, uint32_t base_address) {
  LOG_INFO("==> IPBUS_READ nwords: %i @ addr: %08x\n", nwords, base_address);
  ipbus_payload_t output;
  output.size = nwords;
  output.words = (uint32_t*)malloc(nwords * sizeof(uint32_t));
  for (int i = 0; i < nwords; ++i) {
    output.words[i] = i + 1;
  }
  return output;
}

ipbus_payload_t handle_IPBUS_NIREAD(uint8_t nwords, uint32_t base_address) {
  LOG_INFO("==> IPBUS_NIREAD nwords: %i @ addr: %08x\n", nwords, base_address);
  ipbus_payload_t output;
  output.size = nwords;
  output.words = (uint32_t*)malloc(nwords * sizeof(uint32_t));
  for (int i = 0; i < nwords; ++i) {
    output.words[i] = i + 1;
  }
  return output;
}

// Write functions return no data - return value should indicate whether
// write was successful.  The write address is the first word of the data.
int handle_IPBUS_WRITE(uint8_t writesize, const ipbus_payload_t* data) {
  LOG_INFO("==> IPBUS_WRITE writesize: %i @ addr: %08x\n", writesize, data->words[0]);
  for (int i = 1; i < data->size; ++i) {
    LOG_INFO("====> datum %i: %08x\n", i, data->words[i]);
  }
  return 0;
}

int handle_IPBUS_NIWRITE(uint8_t writesize, const ipbus_payload_t* data) {
  LOG_INFO("==> IPBUS_NIWRITE writesize: %i @ addr: %08x\n", writesize, data->words[0]);
  for (int i = 1; i < data->size; ++i) {
    LOG_INFO("====> datum %i: %08x\n", i, data->words[i]);
  }
  return 0;
}

// Read write modifies returns original contents at address
uint32_t handle_IPBUS_RMW(uint32_t base_address, uint32_t andterm, uint32_t orterm) {
  LOG_INFO("==> IPBUS_RMW @ addr: %08x - AND: %08x OR: %08x\n", 
      base_address, andterm, orterm);
  return (base_address & andterm) | orterm;
}

uint32_t handle_IPBUS_RMWSUM(uint32_t base_address, uint32_t addend) {
  LOG_INFO("==> IPBUS_RMWSUM @ addr: %08x - +: %08x\n", 
      base_address, addend);
  return (base_address + addend);
}
