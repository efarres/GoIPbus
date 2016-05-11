/* tty_functions.h

   Header file for tty_functions.c.

   The source code file is copyright 2010, Michael Kerrisk, and is licensed
   under the GNU Lesser General Public License, version 3.

*/
#ifndef TTY_FUNCTIONS_H
#define TTY_FUNCTIONS_H

#include <termios.h>

int ttySetCbreak(int fd, struct termios *prevTermios);

int ttySetRaw(int fd, struct termios *prevTermios);

#endif
