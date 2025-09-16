#pragma once

#include "hc_evaluation.hpp"
#include "move.hpp"

#include <cstdint>
#include <chrono>

class Board;

struct search_result
{
	std::chrono::milliseconds search_time;
	evaluation::eval_t score;
	Move move;
};

// Use a depth of 0 to search (effectively) infinitely.
search_result get_best_move(const Board &board, uint32_t depth, std::chrono::milliseconds max_time = std::chrono::milliseconds(0));