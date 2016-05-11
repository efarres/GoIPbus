/*
 * =====================================================================================
 *
 *       Filename:  testmembase.c
 *
 *    Description:  Make a dummy block of memory to play with.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <inttypes.h>
#include <stdint.h>
#include <string.h>
//#include <err.h>
#include <sys/mman.h>

#define TESTMEMSIZE 4*1024*1024 // 4MB

#include "macrologger.h"

unsigned char* membase=NULL;

int membase_init(void) {
  membase = (unsigned char*)mmap(
      NULL,     // just map the memory anywhere
      TESTMEMSIZE,     // size of mapped memory
      PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, 
      -1,       // just a block of memory please
      0         //offset
    );
  if (membase == MAP_FAILED) {
    //errx(1, "mmap failed!");
    return 0;
  }
  memset(membase, 0xEF, TESTMEMSIZE);
  LOG_INFO("Memory mapped %i bytes into membase @ %016" PRIxPTR, TESTMEMSIZE, (uintptr_t)membase);
  return 1;
}

int membase_close(void) {
  return 1;
}
