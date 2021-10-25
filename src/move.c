#include "move.h"
#include "board.h"
#include "constants.h"
#include "move_generation.h"

/* Moves piece from start to end and deletes start piece */
static void update_piece(int start, int end) {
    board.colors[end] = board.colors[start];
    board.pieces[end] = board.pieces[start];
    board.colors[start] = EMPTY_COLOR;
    board.pieces[start] = EMPTY_PIECE;
}

/* Changes board based on move (does not check for legality) */
void move_piece(Move move) {
    int square;

    /* Resets ply if pawn move or capture for 50 move rule */
    board.ply++;
    if (board.pieces[move.start] == PAWN || move.flag == CAPTURE) {
        board.ply = 0;
    }

    /* Updates king position */
    if (board.pieces[move.start] == KING) {
        board.king[board.player - 1] = move.end;
    }

    /* Updates board */
    update_piece(move.start, move.end);

    /* Moving rook removes castle option for that side */
    switch (move.start) {
    case H1:
        board.castle -= CASTLE_WK;
        break;
    case A1:
        board.castle -= CASTLE_WQ;
        break;
    case H8:
        board.castle -= CASTLE_BK;
        break;
    case A8:
        board.castle -= CASTLE_BQ;
        break;
    }

    /* Additional moves based on flag */
    switch (move.flag) {
    case DOUBLE:
        board.enpassant = (move.start + move.end) / 2;
        break;
    case ENPASSANT:
        square = 16 * get_rank(move.start) + get_file(move.end);
        board.colors[square] = EMPTY_COLOR;
        board.pieces[square] = EMPTY_PIECE;
        break;
    case CASTLE_WK:
        update_piece(H1, F1);
        board.castle -= CASTLE_WK;
        break;
    case CASTLE_WQ:
        update_piece(A1, D1);
        board.castle -= CASTLE_WQ;
        break;
    case CASTLE_BK:
        update_piece(H8, F8);
        board.castle -= CASTLE_BK;
        break;
    case CASTLE_BQ:
        update_piece(A8, D8);
        board.castle -= CASTLE_BQ;
        break;
    case PROMOTION_N:
        board.pieces[move.end] = PROMOTION_Q;
        break;
    case PROMOTION_B:
        board.pieces[move.end] = PROMOTION_B;
        break;
    case PROMOTION_R:
        board.pieces[move.end] = PROMOTION_R;
        break;
    case PROMOTION_Q:
        board.pieces[move.end] = PROMOTION_Q;
        break;
    }

    /* Removes enpassant flag after 1 move */
    if (move.flag != DOUBLE) {
        board.enpassant = -1;
    }

    /* Switches players */
    board.player = 3 - board.player;
}

/* Reverses move_piece function */
void unmove_piece(Move move) {
    /* Resets ply and enpassant */
    board.ply = move.ply;
    board.enpassant = move.enpassant;

    /* Resets king position */
    if (board.pieces[move.end] == KING) {
        board.king[2 - board.player] = move.start;
    }

    /* Undoes move and replaces any captured piece */
    update_piece(move.end, move.start);
    if (move.captured) {
        /* Replaces piece for enpassant captures */
        if (move.flag == ENPASSANT) {
            int square = 16 * get_rank(move.start) + get_file(move.end);
            board.colors[square] = board.player;
            board.pieces[square] = PAWN;
        } else {
            board.colors[move.end] = board.player;
            board.pieces[move.end] = move.captured;
        }
    }

    /* Additional moves based on flag */
    switch (move.flag) {
    case CASTLE_WK:
        update_piece(F1, H1);
        board.castle += CASTLE_WK;
        break;
    case CASTLE_WQ:
        update_piece(D1, A1);
        board.castle += CASTLE_WQ;
        break;
    case CASTLE_BK:
        update_piece(F8, H8);
        board.castle += CASTLE_BK;
        break;
    case CASTLE_BQ:
        update_piece(D8, A8);
        board.castle += CASTLE_BQ;
        break;
    case PROMOTION_N:
    case PROMOTION_B:
    case PROMOTION_R:
    case PROMOTION_Q:
        board.pieces[move.start] = PAWN;
        break;
    }

    /* Switches players */
    board.player = 3 - board.player;
}

/* Moves piece from start to end if it is legal */
int player_move_piece(Move move) {
    Move moves[MAX_MOVES];
    int count, i;

    /* Start and end must be in the board and be different colors */
    /* Starting square must be the player to move's piece */
    if (invalid_square(move.start) || invalid_square(move.end) ||
        board.colors[move.start] != board.player ||
        board.colors[move.end] == board.player) {
        return FAILURE;
    }

    /* Check if piece is in moves list */
    count = generate_moves(moves);

    /* NOTE: Could change moves from array to hashset later, but this code is
    for checking if player moves are legal, so performance is not critical */

    /* Iterates through all legal moves and checks if the move is in there */
    for (i = 0; i < count; i++) {
        if (moves[i].start == move.start && moves[i].end == move.end) {
            move_piece(move);
            if (is_attacked(board.king[2 - board.player], 3 - board.player)) {
                unmove_piece(move);
                return FAILURE;
            }
            return SUCCESS;
        }
    }

    return FAILURE;
}