#ifndef BOARD_H
#define BOARD_H

#include "types.h"

typedef struct
{
    unsigned int white_king_side : 1;
    unsigned int white_queen_side : 1;
    unsigned int black_king_side : 1;
    unsigned int black_queen_side : 1;
} castling_rights;

typedef struct
{
    bitboard piece_bb[6][2];  // bitboards for each piece type and color [piece][color]
    bitboard all_pieces[2];   // bitboards for all pieces for each color
    castling_rights castling; // castling rights
    enum color side_to_move;  // current side to move
    enum square en_passant;   // en passant square
    int halfmove_clock;       // halfmove clock
    int fullmove_number;      // fullmove number
} board;

void board_init(board *b);

int board_from_fen(board *b, const char *fen);

void board_to_fen(const board *b, char *fen);

void board_print(const board *b);

enum piece board_get_piece_at(const board *b, enum square s);

enum color board_get_color_at(const board *b, enum square s);

int is_square_occuppied(const board *b, enum square s);

void board_move_piece(board *b, enum square from, enum square to);

void is_board_in_check(const board *b, enum color c);

bitboard board_get_attacked_squares(const board *b, enum color c);

#endif