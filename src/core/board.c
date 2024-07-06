#include "board.h"
#include "bitboard.h"
#include <stdio.h>
#include <string.h>

int char_to_digit(char c)
{
    return c - '0';
}

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int is_lower(char c)
{
    return c >= 'a' && c <= 'z';
}

int to_lower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

int string_to_int(const char *str)
{
    int result = 0;
    while (*str && is_digit(*str))
    {
        result = result * 10 + char_to_digit(*str);
        str++;
    }
    return result;
}

void append_char(char **str, char c)
{
    **str = c;
    (*str)++;
}

void append_int(char **str, int n)
{
    if (n == 0)
    {
        append_char(str, '0');
        return;
    }

    char buffer[10];
    int i = 0;
    while (n > 0)
    {
        buffer[i++] = n % 10 + '0';
        n /= 10;
    }

    while (i > 0)
    {
        append_char(str, buffer[--i]);
    }
}

void board_set_piece(board *b, enum square s, enum piece p, enum color c)
{
    bitboard square_bb = 1ULL << s;
    b->piece_bb[p][c] |= square_bb;
    b->all_pieces[c] |= square_bb;
}

void board_remove_piece(board *b, enum square s)
{
    bitboard square_bb = 1ULL << s;
    enum color c = board_get_color_at(b, s);
    enum piece p = board_get_piece_at(b, s);

    if (p != NO_PIECE)
    {
        b->piece_bb[p][c] &= ~square_bb;
        b->all_pieces[c] &= ~square_bb;
    }
}

int board_from_fen(board *b, const char *fen)
{
    board_init(b);

    int rank = 7;
    int file = 0;

    while (*fen && *fen != ' ')
    {
        if (is_digit(*fen))
        {
            file += char_to_digit(*fen);
        }
        else if (*fen == '/')
        {
            rank--;
            file = 0;
        }
        else
        {
            enum piece p = NO_PIECE;
            enum color c = is_lower(*fen) ? BLACK : WHITE;
            switch (to_lower(*fen))
            {
            case 'p':
                p = PAWN;
                break;
            case 'n':
                p = KNIGHT;
                break;
            case 'b':
                p = BISHOP;
                break;
            case 'r':
                p = ROOK;
                break;
            case 'q':
                p = QUEEN;
                break;
            case 'k':
                p = KING;
                break;
            default:
                return 0; // Invalid FEN
            }
            board_set_piece(b, rank * 8 + file, p, c);
            file++;
        }
        fen++;
    }

    if (*fen++ != ' ')
    {
        return 0; // Invalid FEN
    }

    b->side_to_move = *fen == 'w' ? WHITE : BLACK;
    fen += 2;

    b->castling.white_king_side = 0;
    b->castling.white_queen_side = 0;
    b->castling.black_king_side = 0;
    b->castling.black_queen_side = 0;

    while (*fen && *fen != ' ')
    {
        switch (*fen)
        {
        case 'K':
            b->castling.white_king_side = 1;
            break;
        case 'Q':
            b->castling.white_queen_side = 1;
            break;
        case 'k':
            b->castling.black_king_side = 1;
            break;
        case 'q':
            b->castling.black_queen_side = 1;
            break;
        }
        fen++;
    }

    if (*fen++ != ' ')
    {
        return 0; // Invalid FEN
    }

    if (*fen == '-')
    {
        b->en_passant = NO_SQUARE;
        fen += 2;
    }
    else
    {
        file = fen[0] - 'a';
        rank = fen[1] - '1';
        b->en_passant = rank * 8 + file;
        fen += 3;
    }

    b->halfmove_clock = string_to_int(fen);
    while (*fen && *fen != ' ')
    {
        fen++;
    }
    fen++;

    b->fullmove_number = string_to_int(fen);

    return 1;
}

void board_to_fen(const board *b, char *fen)
{
    static const char piece_chars[] = "PNBRQKpnbrqk";
    int empty = 0;

    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            enum square s = rank * 8 + file;
            enum piece p = board_get_piece_at(b, s);
            enum color c = board_get_color_at(b, s);

            if (p == NO_PIECE)
            {
                empty++;
            }
            else
            {
                if (empty)
                {
                    append_char(&fen, empty + '0');
                    empty = 0;
                }
                append_char(&fen, piece_chars[p + (c == BLACK ? 6 : 0)]);
            }
        }

        if (empty)
        {
            append_char(&fen, empty + '0');
            empty = 0;
        }

        if (rank > 0)
        {
            append_char(&fen, '/');
        }
    }

    append_char(&fen, ' ');
    append_char(&fen, (b->side_to_move == WHITE) ? 'w' : 'b');
    append_char(&fen, ' ');

    if (b->castling.white_king_side)
        append_char(&fen, 'K');
    if (b->castling.white_queen_side)
        append_char(&fen, 'Q');
    if (b->castling.black_king_side)
        append_char(&fen, 'k');
    if (b->castling.black_queen_side)
        append_char(&fen, 'q');
    if (!(b->castling.white_king_side || b->castling.white_queen_side ||
          b->castling.black_king_side || b->castling.black_queen_side))
        append_char(&fen, '-');

    append_char(&fen, ' ');

    if (b->en_passant == NO_SQUARE)
    {
        append_char(&fen, '-');
    }
    else
    {
        append_char(&fen, 'a' + (b->en_passant % 8));
        append_char(&fen, '1' + (b->en_passant / 8));
    }

    append_char(&fen, ' ');
    append_int(&fen, b->halfmove_clock);
    append_char(&fen, ' ');
    append_int(&fen, b->fullmove_number);
    append_char(&fen, '\0'); // Null-terminate the string
}

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

    return NO_PIECE;
}

void board_make_move(board *b, enum square from, enum square to)
{
    enum piece moving_piece = board_get_piece_at(b, from);
    enum color moving_color = board_get_color_at(b, from);
    enum piece captured_piece = board_get_piece_at(b, to);
    enum color captured_color = board_get_color_at(b, to);

    // Remove the moving piece from its original square
    board_remove_piece(b, from);

    // If there's a piece on the destination square, remove it
    if (captured_piece != NO_PIECE)
    {
        board_remove_piece(b, to);
    }

    // Place the moving piece on the destination square
    board_set_piece(b, to, moving_piece, moving_color);

    // Handle special pawn moves
    if (moving_piece == PAWN)
    {
        // En passant capture
        if (to == b->en_passant)
        {
            enum square captured_pawn = (moving_color == WHITE) ? to - 8 : to + 8;
            board_remove_piece(b, captured_pawn);
        }

        // Set en passant square if it's a double push
        if (abs((int)from - (int)to) == 16)
        {
            b->en_passant = (enum square)((from + to) / 2);
        }
        else
        {
            b->en_passant = NO_SQUARE;
        }

        // Reset halfmove clock
        b->halfmove_clock = 0;
    }
    else
    {
        // Clear en passant square for non-pawn moves
        b->en_passant = NO_SQUARE;

        // Increment halfmove clock for non-captures
        if (captured_piece == NO_PIECE)
        {
            b->halfmove_clock++;
        }
        else
        {
            b->halfmove_clock = 0;
        }
    }

    // Update castling rights
    if (moving_piece == KING)
    {
        if (moving_color == WHITE)
        {
            b->castling.white_king_side = 0;
            b->castling.white_queen_side = 0;
        }
        else
        {
            b->castling.black_king_side = 0;
            b->castling.black_queen_side = 0;
        }
    }
    else if (moving_piece == ROOK)
    {
        if (from == A1)
            b->castling.white_queen_side = 0;
        else if (from == H1)
            b->castling.white_king_side = 0;
        else if (from == A8)
            b->castling.black_queen_side = 0;
        else if (from == H8)
            b->castling.black_king_side = 0;
    }

    // Switch side to move
    b->side_to_move = (b->side_to_move == WHITE) ? BLACK : WHITE;

    // Increment fullmove number if it's Black's turn
    if (b->side_to_move == WHITE)
    {
        b->fullmove_number++;
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

bitboard knight_attacks(enum square s)
{
    bitboard b = 1ULL << s;
    bitboard attacks = 0;

    attacks |= (b << 17) & ~FILE_A_BB;
    attacks |= (b << 10) & ~(FILE_A_BB | FILE_B_BB);
    attacks |= (b >> 6) & ~(FILE_A_BB | FILE_B_BB);
    attacks |= (b >> 15) & ~FILE_A_BB;
    attacks |= (b << 15) & ~FILE_H_BB;
    attacks |= (b << 6) & ~(FILE_G_BB | FILE_H_BB);
    attacks |= (b >> 10) & ~(FILE_G_BB | FILE_H_BB);
    attacks |= (b >> 17) & ~FILE_H_BB;

    return attacks;
}

bitboard bishop_attacks(enum square s, bitboard occupied)
{
    bitboard attacks = 0;
    int r, f;
    int rk = s / 8;
    int fl = s % 8;

    for (r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= 1ULL << (r * 8 + f);
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }

    return attacks;
}

bitboard rook_attacks(enum square s, bitboard occupied)
{
    bitboard attacks = 0;
    int r, f;
    int rk = s / 8;
    int fl = s % 8;

    for (r = rk + 1; r <= 7; r++)
    {
        attacks |= 1ULL << (r * 8 + fl);
        if (occupied & (1ULL << (r * 8 + fl)))
            break;
    }
    for (r = rk - 1; r >= 0; r--)
    {
        attacks |= 1ULL << (r * 8 + fl);
        if (occupied & (1ULL << (r * 8 + fl)))
            break;
    }
    for (f = fl + 1; f <= 7; f++)
    {
        attacks |= 1ULL << (rk * 8 + f);
        if (occupied & (1ULL << (rk * 8 + f)))
            break;
    }
    for (f = fl - 1; f >= 0; f--)
    {
        attacks |= 1ULL << (rk * 8 + f);
        if (occupied & (1ULL << (rk * 8 + f)))
            break;
    }

    return attacks;
}

bitboard queen_attacks(enum square s, bitboard occupied)
{
    return bishop_attacks(s, occupied) | rook_attacks(s, occupied);
}

bitboard king_attacks(enum square s)
{
    bitboard b = 1ULL << s;
    bitboard attacks = 0;

    attacks |= (b << 8) | (b >> 8);                               // up and down
    attacks |= ((b << 1) & ~FILE_A_BB) | ((b >> 1) & ~FILE_H_BB); // left and right
    attacks |= ((b << 9) & ~FILE_A_BB) | ((b >> 7) & ~FILE_A_BB); // diagonal up
    attacks |= ((b << 7) & ~FILE_H_BB) | ((b >> 9) & ~FILE_H_BB); // diagonal down

    return attacks;
}

bitboard board_get_attacked_squares(const board *b, enum color c)
{
    bitboard attacked_squares = 0;
    bitboard occupied_squares = b->all_pieces[WHITE] | b->all_pieces[BLACK];

    // PAWN attacks
    if (c == WHITE)
    {
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