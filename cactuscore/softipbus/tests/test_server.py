#!/usr/bin/env python

"""

Full scale integration test of soft-ipbus.

Starts a server, connects via uhal, and then reads and writes a
bunch of registers.

"""

import os
import signal
import subprocess
import unittest

import uhal
uhal.setLogLevelTo(uhal.LogLevel.WARNING)


class SoftIPbusTests(unittest.TestCase):
    def setUp(self):
        if os.environ.get('IPBUS_LOCAL', False):
            print "Spawning local soft-ipbus server"
            self.server = subprocess.Popen(['bin/softipbus-test'])
        self.manager = uhal.ConnectionManager(
            "file://etc/test_connections.xml")
        self.hw = self.manager.getDevice('ctp6.test')

    def tearDown(self):
        if os.environ.get('IPBUS_LOCAL', False):
            print "Killing local soft-ipbus server"
            os.kill(self.server.pid, signal.SIGTERM)
        del self.manager
        del self.hw

    def test_readwrite(self):
        n = self.hw.getNode('REG')

        # This depends on the memory just being initialized.
        if os.environ.get('IPBUS_LOCAL', False):
            result = n.read()
            self.assertTrue(not result.valid())
            self.hw.dispatch()
            self.assertTrue(result.valid())
            value = result.value()
            self.assertEqual(value, 0xEFEFEFEF)

        write = n.write(0xABABABAB)
        self.hw.dispatch()
        self.assertTrue(write.valid())

        # can we read it back?
        result = n.read()
        self.hw.dispatch()
        self.assertEqual(result.value(), 0xABABABAB)

        n = self.hw.getNode("MEM")

        print "reading MEM"

        nwords = 20

        newblock = range(nwords)
        n.writeBlock(newblock)
        self.hw.dispatch()

        print "wrote MEM"

        result = n.readBlock(nwords)
        self.hw.dispatch()
        value = list(result)
        self.assertEqual(value, range(nwords))

        print "done"

if __name__ == '__main__':
    unittest.main()
