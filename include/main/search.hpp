#pragma once

#include "move.hpp"

#include <cstdint>

class Board;

// Use a depth of 0 to search (effectively) infinitely.
Move get_best_move(const Board &board, uint32_t depth);