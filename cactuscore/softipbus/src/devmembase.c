/*
 * =====================================================================================
 *
 *       Filename:  devmembase.c
 *
 *    Description:  Use system memory (/dev/mem) as the membase
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <inttypes.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/mman.h>

#include "macrologger.h"

unsigned char* membase=NULL;
static int devmemfd = -1;

int membase_init(void) {
  if ((devmemfd = open("/dev/mem", O_RDWR, 0)) == -1) {
    err(1, "Couldn't open /dev/mem");
    return 0;
  }
  membase = (unsigned char*)mmap(
      NULL,     // just map the memory anywhere
      4096,     // size of mapped memory
      PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, 
      devmemfd,       
      0         //offset
    );
  if (membase == MAP_FAILED) {
    errx(1, "mmap failed!");
    return 0;
  }
  LOG_INFO("Memory mapped /dev/mem into membase @ %016" PRIxPTR "\n", (uintptr_t)membase);
  return 1;
}

int membase_close(void) {
  close(devmemfd);
  return 1;
}
