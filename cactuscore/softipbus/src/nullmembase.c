/*
 * =====================================================================================
 *
 *       Filename:  nullmembase.c
 *
 *    Description:  A membase which does nothing, and is not valid.
 *                  This is used by the forwarding server, which does not 
 *                  require any local memory.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

unsigned char* membase=0;

int membase_init(void) { 
  return 1;
}

int membase_close(void) {
  return 1;
}
