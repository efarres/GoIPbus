#!/usr/bin/env python

"""

Bucky in, Bucky out

"""

import os
import StringIO
import struct

import uhal
uhal.setLogLevelTo(uhal.LogLevel.DEBUG)

def fd_to_word_list(fd):
    # Read a file, and return a list of 32bit integers
    output = []
    four_bytes = fd.read(4)
    while four_bytes != "":
        # pad the end
        if len(four_bytes) < 4:
            four_bytes += " " * (4 - len(four_bytes))
        the_word = struct.unpack("=I", four_bytes)[0]
        output.append(the_word)
        four_bytes = fd.read(4)
    return output

def word_list_2_string(words):
    output = [struct.pack("=I", x) for x in words]
    return "".join(output)



# load name->address mapping for our hardware
manager = uhal.ConnectionManager("file://etc/test_connections.xml")
hw = manager.getDevice('ctp6.test')

node = hw.getNode('MEM')

bucky_in = fd_to_word_list(open('etc/bucky.txt', 'rb'))

print word_list_2_string(bucky_in)

# Send Bucky to the CTP
node.writeBlock(bucky_in)

#hw.dispatch()

# Now read him back
bucky_out = node.readBlock(len(bucky_in))

hw.dispatch()

print word_list_2_string(bucky_out)

