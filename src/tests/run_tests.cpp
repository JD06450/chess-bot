#include "run_tests.hpp"

#include "move_generation_test.hpp"
#include "search_test.hpp"

#include <cxxopts.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	cxxopts::Options program_options("bot_tests",
	                                 "Chess Bot Unit Tests\nRuns unit tests for the chess bot. Specific tests can be "
	                                 "selected or all tests can be ran at once.");

	program_options.allow_unrecognised_options();

	program_options.add_options()("a,all", "Run all tests (Default)")(
	    "m,move-gen",
	    "Run tests for movement generation")("s,search", "Run tests for node searching")("h,help", "Print usage");

	cxxopts::ParseResult result = program_options.parse(argc, argv);

	if (result.unmatched().size() || result.contains("help"))
	{
		std::cout << program_options.help() << std::endl;
		return result.unmatched().size() > 0;
	}

	if (argc == 1 || result.count("all"))
	{
		test_move_generation();
		test_search();
		return 0;
	}

	if (result.count("move-gen")) test_move_generation();
	if (result.count("search")) test_search();
}