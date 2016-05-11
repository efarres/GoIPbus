#!/usr/bin/env python

"""

Full scale integration test of soft-ipbus serial forwarding.

Starts a forwarding server and serial server, connects them w/ a fifo,
connects to the forwarding server via uhal, and then reads and writes a
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
        self.environ = os.environ.copy()

        if os.path.exists('/tmp/test_ipbus_rx_fifo'):
            os.remove('/tmp/test_ipbus_rx_fifo')
        if os.path.exists('/tmp/test_ipbus_tx_fifo'):
            os.remove('/tmp/test_ipbus_tx_fifo')

        os.mkfifo('/tmp/test_ipbus_rx_fifo')
        os.mkfifo('/tmp/test_ipbus_tx_fifo')

        self.forwarding_server = subprocess.Popen(
            ['bin/softipbus-forward-test'],
            env=self.environ)
        self.serial_server = subprocess.Popen(['bin/softipbus-serial',
                                               '/tmp/test_ipbus_tx_fifo',
                                               '/tmp/test_ipbus_rx_fifo'],
                                              env=self.environ)

        self.manager = uhal.ConnectionManager(
            "file://etc/test_connections.xml")
        self.hw = self.manager.getDevice('ctp6.frontend.test')

    def tearDown(self):
        os.kill(self.forwarding_server.pid, signal.SIGINT)
        os.kill(self.serial_server.pid, signal.SIGINT)
        os.remove('/tmp/test_ipbus_rx_fifo')
        os.remove('/tmp/test_ipbus_tx_fifo')
        del self.manager
        del self.hw

    def test_readwrite(self):
        n = self.hw.getNode('REG')

        print "READING"
        result = n.read()
        self.assertTrue(not result.valid())
        self.hw.dispatch()
        self.assertTrue(result.valid())
        value = result.value()
        self.assertEqual(value, 0xEFEFEFEF)

        print "WRITING"
        write = n.write(0xABABABAB)
        self.hw.dispatch()
        self.assertTrue(write.valid())

        # can we read it back?
        print "READBACK"
        result = n.read()
        self.hw.dispatch()
        self.assertEqual(result.value(), 0xABABABAB)

        print "MASKED"
        n = self.hw.getNode('REG_LOWER_MASK')
        write = n.write(0xBABA)
        self.hw.dispatch()

        n = self.hw.getNode("MEM")

        nwords = 20

        newblock = range(nwords)
        print "WRITE MEM"
        n.writeBlock(newblock)
        self.hw.dispatch()

        print "READ MEM"
        result = n.readBlock(nwords)
        self.hw.dispatch()
        value = list(result)
        self.assertEqual(value, range(nwords))

if __name__ == '__main__':
    unittest.main()
