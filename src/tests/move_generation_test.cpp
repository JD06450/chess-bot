#include "move_generation_test.hpp"

#include <chrono>
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

#include "bitboard.hpp"
#include "board.hpp"
#include "fen.hpp"
#include "logger.hpp"
#include "move.hpp"
#include "move_generation.hpp"

constexpr int ply = 4;
void          add_to_debug_dump(const Board &start, const std::vector<Move> &moves);

Logger *logger = new Logger(LOG_LEVEL::DEBUG, "", Logger::HeaderType::NONE);

uint64_t get_num_positions(Board &board, int depth, bool log)
{
	uint64_t total_moves = 0;
	if (depth == 0) return 1;
	std::vector<Move> moves = generate_moves(board);

	if (depth == 1)
	{
		if (log) add_to_debug_dump(board, moves);
		return moves.size();
	}

	for (auto &move : moves)
	{
		board.make_move(move);
		total_moves += get_num_positions(board, depth - 1, log);
		board.unmake_move();
		// bitboard::full_set bb_set = bitboard::generate_bitboard_full_set(board);
		// if (bb_set != board.bitboards) throw std::runtime_error("bitboards out-of-sync after unmove!");
	}

	return total_moves;
}

std::array<size_t, ply> performance_test(Board &start)
{
	using Clock = std::chrono::steady_clock;

	std::array<size_t, ply> results{};
	for (int i = 1; i <= ply; i++)
	{
		std::chrono::time_point begin{ Clock::now() };
		// uint64_t                positions = get_num_positions(start, i, i == ply);
		uint64_t                positions = get_num_positions(start, i, false);
		auto                    end       = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - begin);
		logger->print(LOG_LEVEL::INFO, "Depth: ");
		logger->print(LOG_LEVEL::INFO, std::to_string(i) + " ply   ", TEXT_COLOR::BLUE);
		logger->print(LOG_LEVEL::INFO, "Result: ");
		logger->print(LOG_LEVEL::INFO, std::to_string(positions) + " positions   ", TEXT_COLOR::PURPLE);
		logger->print(LOG_LEVEL::INFO, "Time: ");
		logger->print(LOG_LEVEL::INFO, std::to_string(end.count()) + "ms", TEXT_COLOR::LIGHT_GREEN);
		logger->println(LOG_LEVEL::INFO, "");
		// printf("Depth: %i ply   Result: %lu positions   Time: %lims\n", i, positions, end.count());
		results[i - 1] = positions;
	}
	return results;
}

bool          debug_setup = false;
std::ofstream out_file;
std::mutex    file_mutex;

void setup_debug_dump()
{
	out_file = std::ofstream("./debug_dump.txt");
	if (out_file.is_open() && out_file.good()) debug_setup = true;
}

void add_to_debug_dump(const Board &start, const std::vector<Move> &moves)
{
	if (!debug_setup) return;
	std::lock_guard<std::mutex> file_lock(file_mutex);
	if (!out_file) return;
	if (!out_file.good()) return;
	if (start.moves.size())
	{
		out_file << start.moves.begin()->to_string(start, true);
		for (auto begin = std::next(start.moves.cbegin()); begin != start.moves.cend(); begin++)
			out_file << ',' << begin->to_string(start, true);
	}
	else { out_file << "startpos"; }
	out_file << '\n';

	// out_file << start.to_string();

	// if (!moves.at(0).empty())
	if (moves.size())
	{
		out_file << moves.at(0).to_string(start, true);
		for (auto move = moves.begin() + 1; move != moves.end(); move++)
			out_file << ',' << move->to_string(start, true);
	}
	else { out_file << "no moves"; }
	out_file << "\n\n";
}

void run_test(size_t test_index)
{
	// std::cout << "Testing position " << (test_index + 1) << '\n';
	logger->println(LOG_LEVEL::DEBUG,
	                std::string("Testing position ") + std::to_string(test_index + 1),
	                TEXT_COLOR::NORMAL,
	                true);
	auto result = Board::from_fen(test_positions[test_index]);
	if (!result.has_value())
	{
		std::cout << "Test failed: Failed to generate board.\n";

		logger->print(LOG_LEVEL::ERROR, "Test ", TEXT_COLOR::NORMAL, true);
		logger->print(LOG_LEVEL::ERROR, "failed", TEXT_COLOR::RED, true);
		logger->println(LOG_LEVEL::ERROR, ": Failed to generate board.");
		return;
	}
	Board start = result.value();
	start.update_bitboards();
	std::array<size_t, ply> test_results;

	try
	{
		// start.make_move({ "e1", "g1", move_flags::KINGSIDE_CASTLE });
		// start.make_move({ "a5", "a6" });
		// start.make_move({ "a2", "a3" });
		// start.make_move({ "b4", "a3", move_flags::CAPTURE });
		test_results = performance_test(start);
	}
	catch (const std::exception &e)
	{
		logger->print(LOG_LEVEL::ERROR, "Test ", TEXT_COLOR::NORMAL, true);
		logger->print(LOG_LEVEL::ERROR, "failed", TEXT_COLOR::RED, true);
		logger->print(LOG_LEVEL::ERROR, ": Exception thrown.\n");
		logger->println(LOG_LEVEL::ERROR, e.what());

		return;
	}

	bool failed = false;
	for (int j = 0; j < ply; j++)
	{
		if (test_results[j] == num_positions[test_index][j]) continue;

		// std::cout << "Test failed: Node counts mismatch.\n";
		logger->print(LOG_LEVEL::ERROR, "Test ", TEXT_COLOR::NORMAL, true);
		logger->print(LOG_LEVEL::ERROR, "failed", TEXT_COLOR::RED, true);
		logger->println(LOG_LEVEL::ERROR, ": Node counts mismatch at ply " + std::to_string(j + 1));
		failed = true;
		break;
	}

	if (!failed)
	{
		logger->print(LOG_LEVEL::INFO, "Test ", TEXT_COLOR::NORMAL, true);
		logger->print(LOG_LEVEL::INFO, "Passed", TEXT_COLOR::LIGHT_GREEN, true);
		logger->println(LOG_LEVEL::INFO, "!", TEXT_COLOR::NORMAL, true);
	}
}
void run_all_tests()
{
	for (size_t i = 0; i < test_positions.size(); i++) run_test(i);
}

void test_move_generation()
{
	run_all_tests();
}