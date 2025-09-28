#include "board.hpp"
#include "fen.hpp"
#include <iostream>

int main() {
	// std::cout << "Hello, World!" << std::endl;
	// std::cout << (new Board())->to_string() << std::endl;
	auto board = Board::from_fen(std::string(START_FEN));
	if (!board.has_value()) return 1;
	board->make_move(Move(square_to_index("e2"), square_to_index("e7")));
	std::cout << board->to_string() << std::endl;
	return 0;
}