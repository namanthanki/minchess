#include "board.h"
#include "bitboard.h"
#include <stdio.h>
#include <string.h>

void board_init(board *b)
{
    memset(b, 0, sizeof(board));

    // set all pieces to their starting positions

    // white pieces
    b->piece_bb[PAWN][WHITE] = 0x00FF000000000000ULL;
    b->piece_bb[KNIGHT][WHITE] = 0x4200000000000000ULL;
    b->piece_bb[BISHOP][WHITE] = 0x2400000000000000ULL;
    b->piece_bb[ROOK][WHITE] = 0x8100000000000000ULL;
    b->piece_bb[QUEEN][WHITE] = 0x0800000000000000ULL;
    b->piece_bb[KING][WHITE] = 0x1000000000000000ULL;

    // black pieces
    b->piece_bb[PAWN][BLACK] = 0x000000000000FF00ULL;
    b->piece_bb[KNIGHT][BLACK] = 0x0000000000000042ULL;
    b->piece_bb[BISHOP][BLACK] = 0x0000000000000024ULL;
    b->piece_bb[ROOK][BLACK] = 0x0000000000000081ULL;
    b->piece_bb[QUEEN][BLACK] = 0x0000000000000008ULL;
    b->piece_bb[KING][BLACK] = 0x0000000000000010ULL;

    // all pieces
    for (int piece = PAWN; piece <= KING; piece++)
    {
        b->all_pieces[WHITE] |= b->piece_bb[piece][WHITE];
        b->all_pieces[BLACK] |= b->piece_bb[piece][BLACK];
    }

    b->side_to_move = WHITE;
    b->castling.white_king_side = 1;
    b->castling.white_queen_side = 1;
    b->castling.black_king_side = 1;
    b->castling.black_queen_side = 1;
    b->en_passant = NO_SQUARE;
    b->halfmove_clock = 0;
    b->fullmove_number = 1;
}

void board_print(const board *b)
{
    char piece_chars[] = ".PNBRQKpnbrqk";
    printf("  a b c d e f g h\n");
    for (int rank = 7; rank >= 0; rank--)
    {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++)
        {
            enum square s = rank * 8 + file;
            enum piece p = board_get_piece_at(b, s);
            enum color c = board_get_color_at(b, s);
            if (p != NO_PIECE)
            {
                printf("%c", piece_chars[p + 1 + (c * 6)]);
            }
            else
            {
                printf(". ");
            }
        }
        printf("%d\n", rank + 1);
    }
    printf("  a b c d e f g h\n");
    printf("Side to move: %s\n", b->side_to_move == WHITE ? "white" : "black");
    printf("Castling rights: %c%c%c%c\n",
           b->castling.white_king_side ? 'K' : '-',
           b->castling.white_queen_side ? 'Q' : '-',
           b->castling.black_king_side ? 'k' : '-',
           b->castling.black_queen_side ? 'q' : '-');
    printf("En passant: %c%c\n", b->en_passant == NO_SQUARE ? '-' : 'a' + (b->en_passant % 8), b->en_passant == NO_SQUARE ? '-' : '1' + (b->en_passant / 8));
    printf("Halfmove clock: %d\n", b->halfmove_clock);
    printf("Fullmove number: %d\n", b->fullmove_number);
}

enum piece board_get_piece_at(const board *b, enum square s)
{
    bitboard square_bb = 1ULL << s;
    for (enum piece p = PAWN; p <= KING; p++)
    {
        if (b->piece_bb[p][WHITE] & square_bb || b->piece_bb[p][BLACK] & square_bb)
        {
            return p;
        }
    }
}

enum color board_get_color_at(const board *b, enum square s)
{
    bitboard square_bb = 1ULL << s;
    if (b->all_pieces[WHITE] & square_bb)
    {
        return WHITE;
    }
    if (b->all_pieces[BLACK] & square_bb)
    {
        return BLACK;
    }
    return NO_COLOR;
}

int is_square_occuppied(const board *b, enum square s)
{
    bitboard square_bb = 1ULL << s;
    return (b->all_pieces[WHITE] | b->all_pieces[BLACK]) & square_bb;
}

void board_move_piece(board *b, enum square from, enum square to)
{
    bitboard from_to_bb = (1ULL << from) | (1ULL << to);
    enum piece p = board_get_piece_at(b, from);
    enum color c = board_get_color_at(b, from);
    enum color opponent = c == WHITE ? BLACK : WHITE;

    b->piece_bb[p][c] ^= from_to_bb;
    b->all_pieces[c] ^= from_to_bb;

    if (is_square_occuppied(b, to))
    {
        enum piece captured_piece = board_get_piece_at(b, to);
        b->piece_bb[captured_piece][opponent] &= ~(1ULL << to);
        b->all_pieces[opponent] &= ~(1ULL << to);
    }

    b->side_to_move = opponent;
    b->fullmove_number += (c == BLACK);
    b->halfmove_clock++;

    if (p == PAWN || is_square_occuppied(b, to))
    {
        b->halfmove_clock = 0;
    }

    b->en_passant = NO_SQUARE;

    if (p == PAWN && abs((int)from - (int)to) == 16)
    {
        b->en_passant = (enum square)((from + to) / 2);
    }

    if (from == E1 || to == E1)
    {
        b->castling.white_king_side = 0;
        b->castling.white_queen_side = 0;
    }

    if (from == E8 || to == E8)
    {
        b->castling.black_king_side = 0;
        b->castling.black_queen_side = 0;
    }

    if (from == A1 || to == A1)
    {
        b->castling.white_queen_side = 0;
    }

    if (from == H1 || to == H1)
    {
        b->castling.white_king_side = 0;
    }

    if (from == A8 || to == A8)
    {
        b->castling.black_queen_side = 0;
    }

    if (from == H8 || to == H8)
    {
        b->castling.black_king_side = 0;
    }
}

bitboard board_get_attacked_squares(const board *b, enum color c)
{
    bitboard attacked_squares = 0;
    bitboard occupied_squares = b->all_pieces[WHITE] | b->all_pieces[BLACK];

    // PAWN attacks
    bitboard FILE_A_BB = 0x0101010101010101ULL;
    bitboard FILE_H_BB = 0x8080808080808080ULL;
    if (c == WHITE)
    {
        // where should I define FILE_A_BB and FILE_H_BB?
        attacked_squares |= ((b->piece_bb[PAWN][WHITE] & ~FILE_A_BB) << 7);
        attacked_squares |= ((b->piece_bb[PAWN][WHITE] & ~FILE_H_BB) << 9);
    }
    else
    {
        attacked_squares |= ((b->piece_bb[PAWN][BLACK] & ~FILE_H_BB) >> 7);
        attacked_squares |= ((b->piece_bb[PAWN][BLACK] & ~FILE_A_BB) >> 9);
    }

    // KNIGHT attacks
    bitboard knights = b->piece_bb[KNIGHT][c];
    while (knights)
    {
        enum square s = lsb(knights);
        attacked_squares |= knight_attacks(s);
        knights &= knights - 1;
    }

    // BISHOP and Queens attacks
    bitboard bishops_queens = b->piece_bb[BISHOP][c] | b->piece_bb[QUEEN][c];
    while (bishops_queens)
    {
        enum square s = lsb(bishops_queens);
        attacked_squares |= bishop_attacks(s, occupied_squares);
        bishops_queens &= bishops_queens - 1;
    }

    // ROOK and Queens attacks
    bitboard rooks_queens = b->piece_bb[ROOK][c] | b->piece_bb[QUEEN][c];
    while (rooks_queens)
    {
        enum square s = lsb(rooks_queens);
        attacked_squares |= rook_attacks(s, occupied_squares);
        rooks_queens &= rooks_queens - 1;
    }

    // KING attacks
    bitboard king = b->piece_bb[KING][c];
    attacked_squares |= king_attacks(lsb(king));

    return attacked_squares;
}