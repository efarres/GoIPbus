# Map the oRSC fiber indices to CTP fiber indices

FIBER_MAP = {
    24: 0x5,
    25: 0x4,
    26: 0x8,
    27: 0xb,
    28: 0x6,
    29: 0x7
}

from integration_patterns import pattern as orsc_pattern

def pattern(link):
    if link in FIBER_MAP:
        return orsc_pattern(FIBER_MAP[link]-1)
    return orsc_pattern(link)
