#pragma once

#include <array>
#include <cstdint>

class Board;

namespace evaluation
{

	typedef int32_t eval_t;

namespace hce
{


#pragma region Piece_Values

/**
 * Using a Modified Larry Kaufman's Scoring to make bishops score better than knights
 * Source: https://www.chessprogramming.org/Point_Value#Basic_values
 * Rules:
 * B > N > 3P
 * B + N > R + P
 * B + N = R + 1.5P
 */
namespace piece_values
{

constexpr eval_t KING_MID   = 10'000;
constexpr eval_t QUEEN_MID  = 1000;
constexpr eval_t ROOK_MID   = 525;
constexpr eval_t BISHOP_MID = 360;
constexpr eval_t KNIGHT_MID = 325;
constexpr eval_t PAWN_MID   = 100;

constexpr eval_t KING_END   = 10'000;
constexpr eval_t QUEEN_END  = QUEEN_MID * 1.07;
constexpr eval_t ROOK_END   = ROOK_MID * 1.06;
constexpr eval_t BISHOP_END = BISHOP_MID * 1.05;
constexpr eval_t KNIGHT_END = KNIGHT_MID * 1.03;
constexpr eval_t PAWN_END   = PAWN_MID * 1.08;

} // namespace values

namespace modifier_values
{

constexpr eval_t ROOK_PAIR       = 0;
constexpr eval_t BISHOP_PAIR     = 0;
constexpr eval_t KNIGHT_PAIR     = 0;
constexpr eval_t NO_PAWN_PENALTY = 0;

}

#pragma endregion Piece_Values

#pragma region Piece_Tables

// Due to the layout of the board indices, the tables will be upside-down
// The boards are just ripped straight from the chess programming wiki with some exceptions.
// Might have to mess with these values later...
namespace piece_tables
{
// clang-format off

constexpr std::array<eval_t, 64> PAWNS_MID = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 5, 10, 10,-20,-20, 10, 10,  5,
	 5, -5,-10,  0,  0,-10, -5,  5,
	 0,  0,  0, 20, 20,  0,  0,  0,
	 5,  5, 10, 25, 25, 10,  5,  5,
	10, 10, 20, 30, 30, 20, 10, 10,
	50, 50, 50, 50, 50, 50, 50, 50,
	 0,  0,  0,  0,  0,  0,  0,  0
};

constexpr std::array<eval_t, 64> PAWNS_END = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10,
	20, 20, 20, 20, 20, 20, 20, 20,
	30, 30, 30, 30, 30, 30, 30, 30,
	50, 50, 50, 50, 50, 50, 50, 50,
	 0,  0,  0,  0,  0,  0,  0,  0
};

constexpr std::array<eval_t, 64> KNIGHTS = {
	-50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50
};

constexpr std::array<eval_t, 64> BISHOPS = {
	-20,-10,-10,-10,-10,-10,-10,-20,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-20,-10,-10,-10,-10,-10,-10,-20
};

constexpr std::array<eval_t, 64> ROOKS = {
	 0,  0,  0,  5,  5,  0,  0,  0,
	-5,  0,  0,  0,  0,  0,  0, -5,
	-5,  0,  0,  0,  0,  0,  0, -5,
	-5,  0,  0,  0,  0,  0,  0, -5,
	 0,  0,  0,  0,  0,  0,  0,  0,
	-5,  0,  0,  0,  0,  0,  0, -5,
	 5, 10, 10, 10, 10, 10, 10,  5,
	-5,  0,  0,  0,  0,  0,  0, -5
};

constexpr std::array<eval_t, 64> QUEENS = {
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-10,  5,  5,  5,  5,  5,  0,-10,
	  0,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	-10,  0,  5,  5,  5,  5,  0,-10,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};

constexpr std::array<eval_t, 64> KING_MIDDLE = {
	 20, 30, 10,  0,  0, 10, 30, 20,
	 20, 20,  0,  0,  0,  0, 20, 20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30
};

constexpr std::array<eval_t, 64> KING_END = {
	-50,-30,-30,-30,-30,-30,-30,-50,
	-30,-30,  0,  0,  0,  0,-30,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 30, 40, 40, 30,-10,-30,
	-30,-10, 20, 30, 30, 20,-10,-30,
	-30,-20,-10,  0,  0,-10,-20,-30,
	-50,-40,-30,-20,-20,-30,-40,-50
};

// clang-format on
} // namespace piece_tables

#pragma endregion Piece_Tables

eval_t two_phase_lerp(const Board &state, eval_t p1, eval_t p2);
eval_t four_phase_lerp(const Board &state, eval_t p1, eval_t p2, eval_t p3, eval_t p4);

eval_t evaluate(const Board &state);

} // namespace hce

} // namespace evaluation