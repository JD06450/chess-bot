#include "search.hpp"

#include "board.hpp"
#include "move_generation.hpp"

#include <chrono>
#include <limits>

using evaluation::eval_t;
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

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

search_result get_best_move(const Board &board, uint32_t depth, std::chrono::milliseconds max_time)
{
	search_result result{ 0ms, std::numeric_limits<eval_t>::min(), Move{} };
	std::vector<Move> legal_moves = generate_moves(board);
	Board board_copy = board;
	std::chrono::steady_clock::time_point start = Clock::now(), end;

	for (auto &move : legal_moves)
	{
		eval_t score = -negamax(board.simulate_move(move), depth - 1);
		if (score > result.score)
		{
			result.score = score;
			result.move = move;
		}

		end = Clock::now();
		if (max_time != 0ms && end - start > max_time) break;
	}

	result.search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start + 500us);
	return result;
}