#pragma once

#include <string>

/*
 * A FEN string consists of 6 fields, all separated by a space.
 * The first field is the position of all the PieceType on the board.
 * - Black's PieceType are denoted by lowercase letters, and white uses uppercase.
 * - If one or more spaces are empty, a number is written to specify the number of empty spaces.
 * - The PieceType are listed in row-major order with a slash separating each rank starting from the 8th.
 * The second field indicates whose turn it is. "w" for white, "b" for black.
 * The third field indicates castling rights.
 * - The letters "K" and "Q" are used to specify whether king-side or queen-side castling is allowed, respectively.
 * - White uses uppercase letters and comes first, and black uses lowercase letters and comes second in the order.
 * - If neither side can castle, then a single dash "-" should be used.
 * The fourth field shows which square can be taken by en passant.
 * - If a pawn moves by two squares, then the square behind it is written to this field.
 * - (i.e. e2-e4 -> e3 is the en passant target)
 * - If there is no en passant target, then a single dash "-" should be used.
 * The fifth field counts the number of halfmoves since the last capture or pawn move.
 * The sixth field is the current move number, which starts at 1.
 *
 * The FEN string here shows the starting configuration of a standard chessboard.
 */
const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


// std::unique_ptr<Board> board_from_fen(const std::string& fen_string);

// std::string generate_fen_string(const Board& board);