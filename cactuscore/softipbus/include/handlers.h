/*
 * =====================================================================================
 *
 *       Filename:  handlers.h
 *
 *    Description:  Interface that defines functions to handle IPbus packets.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#ifndef IPBUS_HANDLERS_H
#define IPBUS_HANDLERS_H

#include <stdint.h>
#include "protocol.h"

// Read functions return a buffer of data
ipbus_payload_t handle_IPBUS_READ(uint8_t nwords, uint32_t base_address);
ipbus_payload_t handle_IPBUS_NIREAD(uint8_t nwords, uint32_t base_address);

// Write functions return no data - return value should indicate whether
// write was successful.  The write address is the first word of the data.
int handle_IPBUS_WRITE(uint8_t writesize, const ipbus_payload_t* data);
int handle_IPBUS_NIWRITE(uint8_t writesize, const ipbus_payload_t* data);

// Read write modifies returns original contents at address
uint32_t handle_IPBUS_RMW(uint32_t base_address, uint32_t andterm, uint32_t orterm);
uint32_t handle_IPBUS_RMWSUM(uint32_t base_address, uint32_t addend);

#endif
