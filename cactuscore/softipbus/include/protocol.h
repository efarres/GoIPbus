/*
 * =====================================================================================
 *
 *       Filename:  protocol.h
 *
 *    Description:  Common defintions for the IPbus protocol.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#ifndef IPBUS_PROTOCOL_H
#define IPBUS_PROTOCOL_H


// IPbus control packets are how data is sent/recieved
#define IPBUS_CONTROL_PKT 0x0
// These two packet types are for data reliability over UDP
#define IPBUS_STATUS_PKT 0x1
#define IPBUS_RESEND_PKT 0x2
// not in the protocol
#define IPBUS_PKT_ERR 0x3

#define IPBUS_INFO_SUCCESS 0x0
#define IPBUS_INFO_BADHEADER 0x1
#define IPBUS_INFO_BUSERROR_READ 0x4
#define IPBUS_INFO_BUSERROR_WRITE 0x5
#define IPBUS_INFO_BUSTIMEOUT_READ 0x6
#define IPBUS_INFO_BUSTIMEOUT_WRITE 0x7
#define IPBUS_INFO_REQUEST 0xf

// Transaction types
#define IPBUS_READ 0x0
#define IPBUS_NIREAD 0x2
#define IPBUS_WRITE 0x1
#define IPBUS_NIWRITE 0x3
#define IPBUS_RMW 0x4
#define IPBUS_RMWSUM 0x5

// Input stream states
#define IPBUS_ISTREAM_EMPTY             0x0
#define IPBUS_ISTREAM_PACKET            0x1  // new packet header is in the buffer
#define IPBUS_ISTREAM_PACKET_SWP_ORD    0x2  // new packet header with opposite endianness is in the buffer
#define IPBUS_ISTREAM_PARTIAL_TRANS     0x3  // a partial transaction is in the buffer
#define IPBUS_ISTREAM_FULL_TRANS        0x4  // a full transaction is ready in the buffer
#define IPBUS_ISTREAM_ERR               0xF  // something went wrong dude

#include <stdint.h>

// a buffer of words
typedef struct {
  unsigned char size;
  uint32_t* words;
} ipbus_payload_t;


// an IPbus transaction
typedef struct {
  uint16_t id;
  unsigned char words;
  unsigned char type;
  unsigned char info;
  ipbus_payload_t data;
} ipbus_transaction_t;

#endif
