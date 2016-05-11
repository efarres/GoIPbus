#!/usr/bin/env python

'''

Generates XMD commands for loading a pattern into oRSC block RAMS.

'''

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("pattern", metavar="pattern.py",
                        help="Pattern generator file")
    parser.add_argument("--orbit-capture-char", dest='capture_char',
                        type=str, metavar='0xbc', default='0xbc',
                        help="character to trigger capture "
                        "- default: %(default)s")
    parser.add_argument("--sop-char", dest='sop_char',
                        type=str, metavar='0xbc', default='0xbc',
                        help="character to indicating start of packet"
                        "- default: %(default)s")
    parser.add_argument("--cycles", type=int, metavar='N', default=1,
                        help="number of patterns to repeat in orbit"
                        "- default: %(default)s")

    args = parser.parse_args()

    mod = __import__(args.pattern.replace('.py', ''))

    pattern = getattr(mod, 'pattern')

    for link in range(48):
        base_addr = 0x10000000 + 0x1000 * link
        data = pattern(link)
        idx = 0
        for cycle in range(args.cycles):
            for word in data:
                addr = base_addr + 0x4 * idx
                print "mwr 0x%08x 0x%08x" % (addr, word)
                idx += 1
