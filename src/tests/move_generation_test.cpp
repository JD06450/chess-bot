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

const std::string test_position_2 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const std::string test_position_3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
const std::string test_position_4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
const std::string test_position_5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
const std::string test_position_6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

const std::array<std::string, 6> test_positions{ START_FEN,       test_position_2, test_position_3,
	                                             test_position_4, test_position_5, test_position_6 };

const std::array<std::array<size_t, 5>, test_positions.size()> num_positions(
    { { { 20, 400, 8902, 197'281, 4'865'609 } },
      { { 48, 2039, 97'862, 4'085'603, 193'690'690 } },
      { { 14, 191, 2812, 43'238, 674'624 } },
      { { 6, 264, 9467, 422'333, 15'833'292 } },
      { { 44, 1486, 62'379, 2'103'487, 89'941'194 } },
      { { 46, 2079, 89'890, 3'894'594, 164'075'551 } } });

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
int main()
{
	run_all_tests();

	// setup_debug_dump();
	// run_test(1);
	// Logger::print_color_test();
}