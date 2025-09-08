#include "logger.hpp"

#include <chrono>

constexpr char ESC_SEQ_MARK_BOLD[] = "\033[1m";
constexpr char ESC_SEQ_MARK_RESET[] = "\033[0m";

std::string pad_spaces(const std::string &input, size_t desired_length)
{
	if (input.length() >= desired_length)
		return input;

	size_t zerosToAdd = desired_length - input.length();

	std::string padding(zerosToAdd, ' ');

	return padding + input;
}

std::string trunc_zeros(const std::string &input, size_t num_zeros)
{
	auto point_position = input.find('.');
	if (point_position == std::string::npos)
		return input;
	return input.substr(0, std::min(point_position + num_zeros, input.length()));
}

constexpr std::string log_level_to_string(LOG_LEVEL level)
{
	switch (level)
	{
	case LOG_LEVEL::DEBUG:
		return "DEBUG";
	case LOG_LEVEL::INFO:
		return "INFO";
	case LOG_LEVEL::ERROR:
		return "ERROR";
	case LOG_LEVEL::CRITICAL:
		return "CRITICAL";
	case LOG_LEVEL::FATAL:
		return "FATAL";
	case LOG_LEVEL::WARNING:
	default:
		return "WARNING";
	}
}

constexpr std::string text_color_to_string(TEXT_COLOR color)
{
	switch (color) {
	case TEXT_COLOR::DEBUG: return log_level_to_string(LOG_LEVEL::DEBUG);
	case TEXT_COLOR::INFO: return log_level_to_string(LOG_LEVEL::INFO);
	case TEXT_COLOR::WARNING: return log_level_to_string(LOG_LEVEL::WARNING);
	case TEXT_COLOR::ERROR: return log_level_to_string(LOG_LEVEL::ERROR);
	case TEXT_COLOR::CRITICAL: return log_level_to_string(LOG_LEVEL::CRITICAL);
	case TEXT_COLOR::FATAL: return log_level_to_string(LOG_LEVEL::FATAL);

	case TEXT_COLOR::RED: return "RED";
	case TEXT_COLOR::DARK_RED: return "DARK RED";
	case TEXT_COLOR::ORANGE: return "ORANGE";
	case TEXT_COLOR::YELLOW: return "YELLOW";
	case TEXT_COLOR::LIGHT_GREEN: return "LIGHT GREEN";
	case TEXT_COLOR::GREEN: return "GREEN";
	case TEXT_COLOR::CYAN: return "CYAN";
	case TEXT_COLOR::LIGHT_BLUE: return "LIGHT BLUE";
	case TEXT_COLOR::BLUE: return "BLUE";
	case TEXT_COLOR::PURPLE: return "PURPLE";
	case TEXT_COLOR::PINK: return "PINK";
	case TEXT_COLOR::BROWN: return "BROWN";

	case TEXT_COLOR::WHITE: return "WHITE";
	case TEXT_COLOR::LIGHT_GRAY: return "LIGHT_GRAY";
	case TEXT_COLOR::GRAY: return "GRAY";
	case TEXT_COLOR::DARK_GRAY: return "DARK_GRAY";
	case TEXT_COLOR::BLACK: return "BLACK";
	
	case TEXT_COLOR::NORMAL:
	default: return "NORMAL";
	}
}

constexpr std::string log_level_to_escape_seq(LOG_LEVEL level)
{
	switch (level)
	{
	case LOG_LEVEL::DEBUG:
		// gray
		return "\033[38;5;248m";
	case LOG_LEVEL::WARNING:
		// yellow
		return "\033[38;5;220m";
	case LOG_LEVEL::ERROR:
		// red
		return "\033[38;5;9m";
	case LOG_LEVEL::CRITICAL:
		// dark red
		return "\033[38;5;124m";
	case LOG_LEVEL::FATAL:
		// white on red
		return "\033[37;41m";
	case LOG_LEVEL::INFO:
	default:
		// white
		return "\033[97m";
	}
}

constexpr std::string text_color_to_escape_seq(TEXT_COLOR color)
{
	switch (color)
	{
	case TEXT_COLOR::DEBUG: return log_level_to_escape_seq(LOG_LEVEL::DEBUG);
	case TEXT_COLOR::INFO: return log_level_to_escape_seq(LOG_LEVEL::INFO);
	case TEXT_COLOR::WARNING: return log_level_to_escape_seq(LOG_LEVEL::WARNING);
	case TEXT_COLOR::ERROR: return log_level_to_escape_seq(LOG_LEVEL::ERROR);
	case TEXT_COLOR::CRITICAL: return log_level_to_escape_seq(LOG_LEVEL::CRITICAL);
	case TEXT_COLOR::FATAL: return log_level_to_escape_seq(LOG_LEVEL::FATAL);

	case TEXT_COLOR::RED: return "\033[38;5;9m";
	case TEXT_COLOR::DARK_RED: return "\033[38;5;88m";
	case TEXT_COLOR::ORANGE: return "\033[38;5;208m";
	case TEXT_COLOR::YELLOW: return "\033[38;5;220m";
	case TEXT_COLOR::LIGHT_GREEN: return "\033[38;5;10m";
	case TEXT_COLOR::GREEN: return "\033[38;5;28m";
	case TEXT_COLOR::CYAN: return "\033[38;5;51m";
	case TEXT_COLOR::LIGHT_BLUE: return "\033[38;5;81m";
	case TEXT_COLOR::BLUE: return "\033[38;5;12m";
	case TEXT_COLOR::PURPLE: return "\033[38;5;93m";
	case TEXT_COLOR::PINK: return "\033[38;5;207m";
	case TEXT_COLOR::BROWN: return "\033[38;5;94m";
	
	case TEXT_COLOR::WHITE: return "\033[38;5;15m";
	case TEXT_COLOR::LIGHT_GRAY: return "\033[38;5;250m";
	case TEXT_COLOR::GRAY: return "\033[38;5;244m";
	case TEXT_COLOR::DARK_GRAY: return "\033[38;5;236m";
	case TEXT_COLOR::BLACK: return "\033[38;5;0m";
	
	case TEXT_COLOR::NORMAL:
	default: return ESC_SEQ_MARK_RESET;
	// break;
	}
}

std::string Logger::get_timestamp()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	// Convert the time point to a time_t type (Unix timestamp)
	std::time_t current_time = std::chrono::system_clock::to_time_t(now);

	// Convert the time_t to a tm struct for formatting
	std::tm time_info;
	localtime_r(&current_time, &time_info);

	// Extract individual components (month, day, year, hour, minute, second, millisecond)
	int month = time_info.tm_mon + 1; // tm_mon is zero-based
	int day = time_info.tm_mday;
	int year = time_info.tm_year + 1900; // tm_year is years since 1900
	int hour = time_info.tm_hour;
	int minute = time_info.tm_min;
	int second = time_info.tm_sec;
	std::string time_zone(time_info.tm_zone);

	// Get the fractional part in milliseconds of the current time in seconds.
	long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

	// Format the time in this format: mm/dd/yyyy hh/mm/ss.mmm
	std::stringstream formatted_time;
	formatted_time << std::setfill('0');
	formatted_time << std::setw(2) << month << "/";
	formatted_time << std::setw(2) << day << "/";
	formatted_time << year << " ";
	formatted_time << std::setw(2) << hour << ":";
	formatted_time << std::setw(2) << minute << ":";
	formatted_time << std::setw(2) << second << ".";
	formatted_time << std::setw(3) << milliseconds << " ";
	formatted_time << time_zone;
	return formatted_time.str();
}

std::string set_color(const std::string &text, TEXT_COLOR color)
{
	return text_color_to_escape_seq(color) + text;
}

void Logger::print_color_test()
{
	const std::string light_green_dash = set_color("-", TEXT_COLOR::LIGHT_GREEN);
	const std::string green_ddash = set_color("=", TEXT_COLOR::GREEN);
	std::cout << ESC_SEQ_MARK_BOLD << light_green_dash << green_ddash << light_green_dash << green_ddash << light_green_dash;
	std::cout << set_color(" Logger Color Test ", TEXT_COLOR::WHITE);
	std::cout << ESC_SEQ_MARK_BOLD << light_green_dash << green_ddash << light_green_dash << green_ddash << light_green_dash;
	std::cout << "\n";

	for (int c = (int) TEXT_COLOR::NORMAL; c < (int) TEXT_COLOR::MAX_COLORS; c++)
	{
		std::cout << text_color_to_escape_seq((TEXT_COLOR) c) << text_color_to_string((TEXT_COLOR) c) << ESC_SEQ_MARK_RESET << "\n";
	}
	std::cout << "\n";
}

void Logger::print_header(LOG_LEVEL level)
{
	std::string time = Logger::get_timestamp();

	if (this->header_level == HeaderType::FULL) std::cout << "[ " << time << " ]";
	if (this->header_level >= HeaderType::SHORT)
	{
		if (this->label != "") std::cout << "[ " << this->label << " ]";
		std::cout << "[ " << ESC_SEQ_MARK_BOLD << log_level_to_escape_seq(level) << log_level_to_string(level) << ESC_SEQ_MARK_RESET << " ]: ";
	}
}

void Logger::print(LOG_LEVEL level, const std::string &output, TEXT_COLOR color, bool bold)
{
	if (level < this->log_level) return;
	if (this->_print_header)
	{
		if (this->header_level > HeaderType::NONE) this->print_header(level);
		this->_print_header = false;
	}
	std::cout << set_color((bold ? ESC_SEQ_MARK_BOLD : "") + output, color) << ESC_SEQ_MARK_RESET;
};

void Logger::println(LOG_LEVEL level, const std::string &output, TEXT_COLOR color, bool bold)
{
	if (level < this->log_level) return;
	std::string time = Logger::get_timestamp();

	if (this->_print_header && this->header_level > HeaderType::NONE) this->print_header(level);
	else if (!this->_print_header) this->_print_header = true;

	if (output == "") { std::cout << "\n"; return; }
	std::cout << set_color((bold ? ESC_SEQ_MARK_BOLD : "") + output, color) << ESC_SEQ_MARK_RESET << '\n';
}