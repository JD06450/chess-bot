#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <cstddef>

#include "bitboard.hpp"
#include "move.hpp"

// https://www.chessprogramming.org/Encoding_Moves#MoveIndex
// The real number is 218, but rounding up to the nearest power of two here for padding.
// If performance becomes an issue, this can be adjusted.
constexpr size_t MAX_MOVES_PER_BOARD = 256;

static consteval std::array<std::array<size_t, 8>, 64> _precompute_squares_to_edge()
{
	std::array<std::array<size_t, 8>, 64> precomputed_edges;

	for (int rank = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			std::size_t square_idx = rank * 8 + file;

			size_t num_up    = 7 - rank;
			size_t num_down  = rank;
			size_t num_left  = file;
			size_t num_right = 7 - file;

			precomputed_edges.at(square_idx) = { num_up,
				                                 num_down,
				                                 num_left,
				                                 num_right,

				                                 std::min(num_up, num_left),
				                                 std::min(num_up, num_right),
				                                 std::min(num_down, num_left),
				                                 std::min(num_down, num_right) };
		}
	}

	return precomputed_edges;
}

constexpr std::array<std::array<size_t, 8>, 64> NUM_SQUARES_TO_EDGE = _precompute_squares_to_edge();

static consteval std::array<std::array<bitboard::bitboard, 2>, 64> _precompute_pawn_squares(DirectionOffset push,
                                                                                            int double_push_rank)
{
	std::array<std::array<bitboard::bitboard, 2>, 64> moves{};

	for (int from_square = 0; from_square < moves.size(); from_square++)
	{
		uint16_t to        = from_square + (int) push;
		uint16_t double_to = to + (int) push;

		bool push_valid        = inside_board(to);
		bool double_push_valid = push_valid && get_rank_from_square(from_square) == double_push_rank;

		// Minor optimization. Don't need to check color of pawn since a white pawn can't be pushed to the first rank,
		// and a black pawn can't be pushed to the eighth rank.
		// bool is_promotion = (get_rank_from_square(to) == 7) || (get_rank_from_square(to) == 0);

		if (push_valid) moves[from_square][0].set(to);
		if (double_push_valid) moves[from_square][1].set(double_to);
	}

	return moves;
}

static consteval std::array<bitboard::bitboard, 64> _precompute_pawn_captures(DirectionOffset left,
                                                                              DirectionOffset right)
{
	std::array<bitboard::bitboard, 64> captures{};

	for (int from_square = 0; from_square < 64; from_square++)
	{
		int from_file = get_file_from_square(from_square);

		uint16_t left_to          = from_square + (int) left;
		int      left_edge_check  = 0;
		uint16_t right_to         = from_square + (int) right;
		int      right_edge_check = 7;

		bool left_is_valid  = inside_board(left_to) && from_file != left_edge_check;
		bool right_is_valid = inside_board(right_to) && from_file != right_edge_check;

		if (left_is_valid) captures[from_square].set(left_to);
		if (right_is_valid) captures[from_square].set(right_to);
	}

	return captures;
}

constexpr std::array<std::array<bitboard::bitboard, 2>, 64> WHITE_PAWN_MOVES = _precompute_pawn_squares(
    DirectionOffset::UP,
    1);
constexpr std::array<bitboard::bitboard, 64> WHITE_PAWN_CAPTURES = _precompute_pawn_captures(DirectionOffset::UP_LEFT,
                                                                                             DirectionOffset::UP_RIGHT);
constexpr std::array<std::array<bitboard::bitboard, 2>, 64> BLACK_PAWN_MOVES = _precompute_pawn_squares(
    DirectionOffset::DOWN,
    6);
constexpr std::array<bitboard::bitboard, 64> BLACK_PAWN_CAPTURES = _precompute_pawn_captures(
    DirectionOffset::DOWN_LEFT,
    DirectionOffset::DOWN_RIGHT);

static consteval std::array<bitboard::bitboard, 64> _precompute_knight_squares()
{
	std::array<bitboard::bitboard, 64> moves{};

	for (size_t from_square = 0; from_square < 64; from_square++)
	{
		for (auto &dir : KNIGHT_DIRECTION_OFFSETS)
		{
			const int to_square = from_square + dir;

			int file_diff = get_file_from_square(from_square) - get_file_from_square(to_square);
			int rank_diff = get_rank_from_square(from_square) - get_rank_from_square(to_square);

			// Forced to do this because std::abs isn't constexpr. >:(
			file_diff = file_diff < 0 ? -file_diff : file_diff;
			rank_diff = rank_diff < 0 ? -rank_diff : rank_diff;

			bool valid_target = file_diff <= 2 && rank_diff <= 2;
			if (!inside_board(to_square)) continue;
			moves[from_square].set(to_square, valid_target);
		}
	}

	return moves;
}

constexpr std::array<bitboard::bitboard, 64> KNIGHT_MOVES = _precompute_knight_squares();

static consteval std::array<bitboard::bitboard, 64> _precompute_king_squares()
{
	std::array<bitboard::bitboard, 64> moves{};

	for (int from_square = 0; from_square < 64; from_square++)
		for (auto offset : DIRECTION_OFFSETS)
			if (inside_board(from_square + (int) offset)) moves[from_square].set(from_square + (int) offset);

	return moves;
}

constexpr std::array<bitboard::bitboard, 64> KING_MOVES = _precompute_king_squares();

std::vector<Move> generate_moves(const Board &state);