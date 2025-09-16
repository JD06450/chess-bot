#include "search_test.hpp"

#include "board.hpp"
#include "fen.hpp"
#include "search.hpp"

#include <exception>
#include <iostream>
#include <optional>
#include <sstream>

#include "logger.hpp"

constexpr int depth = 4;
Logger *search_logger = new Logger(LOG_LEVEL::DEBUG, "Search Test", Logger::HeaderType::SHORT);

void print_test_result(const search_result &result, const Board &state)
{
	search_logger->println(LOG_LEVEL::INFO, "Search Result:", TEXT_COLOR::WHITE, true);
	search_logger->print(LOG_LEVEL::INFO, "    Move: ");
	search_logger->println(LOG_LEVEL::INFO, result.move.to_string(state, true), TEXT_COLOR::BLUE);
	search_logger->print(LOG_LEVEL::INFO, "    Eval: ");
	search_logger->println(LOG_LEVEL::INFO, std::to_string(result.score), TEXT_COLOR::PURPLE);
	search_logger->print(LOG_LEVEL::INFO, "    Time: ");
	search_logger->println(LOG_LEVEL::INFO, std::to_string(result.search_time.count()) + "ms", TEXT_COLOR::LIGHT_GREEN);
}

search_result run_test_for_position(std::string fen_string)
{
	std::optional<Board> result = Board::from_fen(fen_string);
	if (!result.has_value())
	{
		search_logger->error("Board failed to generate!");
		return search_result{};
	}

	Board new_board = result.value();
	new_board.update_bitboards();

	search_result test_result = get_best_move(new_board, depth);

	print_test_result(test_result, new_board);
	return test_result;
}

void test_search()
{
	for (auto &position : test_positions)
	{
		std::ostringstream search_start_string;
		search_start_string << "Searching position at depth " << depth << ": " << position << "...";
		search_logger->println(LOG_LEVEL::INFO, search_start_string.str(), TEXT_COLOR::WHITE, true);

		try
		{
			(void) run_test_for_position(position);
		}
		catch (std::exception &e)
		{
			search_logger->print(LOG_LEVEL::ERROR, "Test ", TEXT_COLOR::NORMAL, true);
			search_logger->print(LOG_LEVEL::ERROR, "failed", TEXT_COLOR::RED, true);
			search_logger->print(LOG_LEVEL::ERROR, ": Exception thrown.\n");
			search_logger->println(LOG_LEVEL::ERROR, e.what());
		}
	}
}