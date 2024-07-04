#include "bitboard.h"
#include <stdio.h>

bitboard count_trailing_zeros(bitboard bb)
{
    unsigned int count = 0;
    if (bb == 0)
    {
        return 64;
    }
    while (!(bb & 0x1))
    {
        bb >>= 1;
        count++;
    }
    return count;
}

bitboard set_bit(bitboard bb, enum square s)
{
    return bb | (1ULL << s);
}

bitboard clear_bit(bitboard bb, enum square s)
{
    return bb & ~(1ULL << s);
}

int get_bit(bitboard bb, enum square s)
{
    return (bb & (1ULL << s)) != 0;
}

int pop_count(bitboard bb)
{
    int count = 0;
    while (bb)
    {
        count++;
        bb &= bb - 1;
    }
    return count;
}

enum square lsb(bitboard bb)
{
    if (bb == 0)
        return NO_SQUARE;
    return (enum square)count_trailing_zeros(bb);
}

enum square msb(bitboard bb)
{
    if (bb == 0)
        return NO_SQUARE;
    return (enum square)(A8 - count_trailing_zeros(bb));
}

void bitboard_to_string(bitboard bb, char *str)
{
    int index = 0;
    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            enum square s = (enum square)(rank * 8 + file);
            str[index++] = get_bit(bb, s) ? '1' : '0';
            str[index++] = ' ';
        }
        str[index++] = '\n';
    }
    str[index] = '\0';
}