#ifndef BOARD_H
#define BOARD_H

#include "types.h"

// File masks
#define FILE_A_BB 0x0101010101010101ULL
#define FILE_B_BB 0x0202020202020202ULL
#define FILE_C_BB 0x0404040404040404ULL
#define FILE_D_BB 0x0808080808080808ULL
#define FILE_E_BB 0x1010101010101010ULL
#define FILE_F_BB 0x2020202020202020ULL
#define FILE_G_BB 0x4040404040404040ULL
#define FILE_H_BB 0x8080808080808080ULL

// Rank masks
#define RANK_1_BB 0x00000000000000FFULL
#define RANK_2_BB 0x000000000000FF00ULL
#define RANK_3_BB 0x0000000000FF0000ULL
#define RANK_4_BB 0x00000000FF000000ULL
#define RANK_5_BB 0x000000FF00000000ULL
#define RANK_6_BB 0x0000FF0000000000ULL
#define RANK_7_BB 0x00FF000000000000ULL
#define RANK_8_BB 0xFF00000000000000ULL

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

bitboard knight_attacks(enum square s);
bitboard bishop_attacks(enum square s, bitboard occupied);
bitboard rook_attacks(enum square s, bitboard occupied);
bitboard queen_attacks(enum square s, bitboard occupied);
bitboard king_attacks(enum square s);
bitboard board_get_attacked_squares(const board *b, enum color c);

#endif