/*
 * =====================================================================================
 *
 *       Filename:  endian.h
 *
 *    Description:  Portability layer around endian-ness conversions
 *
 *         Author:  Evan Friis, evan.friis@cern.ch UW Madison
 *
 * =====================================================================================
 */

#ifndef ENDIAN_PY41DJGO
#define ENDIAN_PY41DJGO

#if defined(__linux__)

#include <byteswap.h>
#include <arpa/inet.h>
#define host_to_network(x) htonl(x)
#define network_to_host(x) ntohl(x)

#else

#define	bswap_16(value)  \
  ((((value) & 0xff) << 8) | ((value) >> 8))

#define	bswap_32(value)	\
  (((uint32_t)bswap_16((uint16_t)((value) & 0xffff)) << 16) | \
   (uint32_t)bswap_16((uint16_t)((value) >> 16)))

#define	bswap_64(value)	\
  (((uint64_t)bswap_32((uint32_t)((value) & 0xffffffff)) \
    << 32) | \
   (uint64_t)bswap_32((uint32_t)((value) >> 32)))

#define __bswap_16 bswap_16
#define __bswap_32 bswap_32
#define __bswap_64 bswap_64

#ifdef LITTLE_ENDIAN
#define host_to_network(x) bswap_32(x)
#define network_to_host(x) bswap_32(x)
#else
#define host_to_network(x) (x)
#define network_to_host(x) (x)
#endif

#endif

#endif /* end of include guard: ENDIAN_PY41DJGO */

