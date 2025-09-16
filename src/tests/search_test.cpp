#include "search_test.hpp"

#include "board.hpp"
#include "fen.hpp"
#include "search.hpp"

#include <iostream>
#include <optional>

void test_search()
{
	std::optional<Board> result = Board::from_fen(START_FEN);

	if (!result.has_value())
	{
		std::cout << "Board failed to generate" << std::endl;
		return;
	}

	Board new_board = result.value();
	new_board.update_bitboards();

	Move best_move = get_best_move(new_board, 4);
}