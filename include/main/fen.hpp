#pragma once

#include <array>
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

const std::string test_position_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const std::string test_position_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
const std::string test_position_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
const std::string test_position_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
const std::string test_position_6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

const std::array<std::string, 6> test_positions{ START_FEN,       test_position_2, test_position_3,
	                                             test_position_4, test_position_5, test_position_6 };

const std::array<const std::array<const size_t, 5>, test_positions.size()> num_positions(
    { { { 20, 400, 8902, 197'281, 4'865'609 } },
      { { 48, 2039, 97'862, 4'085'603, 193'690'690 } },
      { { 14, 191, 2812, 43'238, 674'624 } },
      { { 6, 264, 9467, 422'333, 15'833'292 } },
      { { 44, 1486, 62'379, 2'103'487, 89'941'194 } },
      { { 46, 2079, 89'890, 3'894'594, 164'075'551 } } });