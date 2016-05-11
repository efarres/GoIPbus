SIZE = 16

CAPTURE_CHAR = 0x3C     # K28.1
SOP_CHAR = 0xBC         # K28.5

def pattern(link):
    base_pattern = ((link + 1) << 8)
    # per imperial grps request
    base_pattern = 0
    output = []
    for i in range(SIZE):
        index_pattern = (i << 16)
        word = base_pattern | index_pattern
        if i == 0:
            word = (word & 0xFFFFFF00) | CAPTURE_CHAR
        elif i % 4 == 0:
            word = 0x505050bc
        output.append(word)
    return output
