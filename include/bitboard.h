#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

bitboard count_trailing_zeros(bitboard bb);

bitboard set_bit(bitboard bb, enum square s);
bitboard clear_bit(bitboard bb, enum square s);

int get_bit(bitboard bb, enum square s);
int pop_count(bitboard bb);

enum square lsb(bitboard bb);
enum square msb(bitboard bb);

void bitboard_to_string(bitboard bb, char *str);

#endif