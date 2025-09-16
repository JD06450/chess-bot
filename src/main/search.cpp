#include "search.hpp"

#include "board.hpp"
#include "hc_evaluation.hpp"
#include "move_generation.hpp"

#include <limits>
#include <numeric>

using evaluation::eval_t;

eval_t negamax(Board &&board, uint32_t depth)
{
	if (depth == 0) return evaluation::hce::evaluate(board);
	eval_t max = std::numeric_limits<eval_t>::min();

	std::vector<Move> legal_moves = generate_moves(board);

	for (auto &move : legal_moves)
	{
		board.make_move(move);
		eval_t score = -negamax(std::forward<Board>(board), depth - 1);
		board.unmake_move();
		if (score > max) max = score;
	}

	return max;
}

Move get_best_move(const Board &board, uint32_t depth)
{
	std::vector<Move> legal_moves = generate_moves(board);

	Move   best_move{};
	eval_t best_score = 0;

	Board board_copy = board;

	for (auto &move : legal_moves)
	{
		eval_t score = -negamax(board.simulate_move(move), depth - 1);
		if (score > best_score)
		{
			best_score = score;
			best_move  = move;
		}
	}

	return best_move;
}