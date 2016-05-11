/*
 * =====================================================================================
 *
 *       Filename:  membase.h
 *
 *    Description:  Defintion of pointer to base address that memory will be
 *                  read reference to.  One shared lib must allocate this.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#ifndef IPBUS_MEMBASE_H
#define IPBUS_MEMBASE_H

extern unsigned char* membase;

// initialize membase variable
int membase_init(void);

// cleanup membase variable
int membase_close(void);

#endif
