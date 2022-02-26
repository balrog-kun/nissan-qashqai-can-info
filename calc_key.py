def calc_key(i):
    val = ((i >> 8) + 0x97) & 0xff
    offset = val >> 6
    calc = (val * 5 + offset) & 0xff
    special_column = calc & 0xf
    bit10_lut = 0xbbbc447bba444bbb5552aa9554aab555888f77488b7768883334ccb330ccd333
    bit11_lut = 0x3730c8c8c9373737d8d8d8e7272738d8dadd252526dadada6565651a9a9a8565

    bit8 = ((i >> 8) ^ (i >> 4) ^ i ^ offset ^ (calc >> 4) ^ (1 if (i & 0xf) == special_column else 0)) & 1
    bit9 = ((i >> 9) ^ (i >> 5) ^ ((i + 1) >> 1) ^ (offset >> 1) ^ (calc >> 5) ^ (1 ^ ((i ^ calc) >> 4) if (i & 0xf) == special_column else 0)) & 1
    bit9 ^= 0 if (offset & 1) and (i & 0x100) else 1
    bit10 = ((i >> 10) ^ (i >> 6) ^ (i >> 2) ^ (1 if (i & 0xf) == special_column and (i ^ calc) & 0x30 == 0 else 0)) & 1
    bit10 ^= 1 if (i ^ ((i >> 8) + offset + 3)) & 3 == 0 else 0
    bit10 ^= bit10_lut >> (255 - val) & 1
    bit11 = ((i >> 11) ^ (i >> 3) ^ ((i + 0x40) >> 7)) & 1
    bit11 ^= 1 if (i ^ special_column) & 7 == 0 and ((i ^ calc ^ 0x40) & 0x70 or (i ^ special_column) & 0xf) else 0
    bit11 ^= bit11_lut >> (255 - val) & 1

    nibble34 = (bit11 << 3) | (bit10 << 2) | (bit9 << 1) | bit8
    return i ^ ((nibble34 << 12) | (nibble34 << 8) | calc)
