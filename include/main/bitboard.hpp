#pragma once

#include "move.hpp"
#include "pieces.hpp"
// #include <bitset>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <vector>

class Board;

namespace bitboard
{

class bitboard
{
public:
	std::uint64_t bits = 0;
	// clang-format off
	constexpr bitboard() noexcept : bits(0) {}
	constexpr bitboard(std::uint64_t n) noexcept : bits(n) {}

	constexpr bool operator==(const bitboard &rhs) const noexcept { return bits == rhs.bits; }
	constexpr bool operator!=(const bitboard &rhs) const noexcept { return bits != rhs.bits; }
	constexpr bool operator<=(const bitboard &rhs) const noexcept { return bits <= rhs.bits; }
	constexpr bool operator>=(const bitboard &rhs) const noexcept { return bits >= rhs.bits; }
	constexpr bool operator<(const bitboard &rhs) const noexcept { return bits < rhs.bits; }
	constexpr bool operator>(const bitboard &rhs) const noexcept { return bits > rhs.bits; }
	constexpr auto operator<=>(const bitboard &rhs) const noexcept { return bits <=> rhs.bits; }

	// constexpr operator bool() const { return bits > 0; }

	constexpr bitboard &operator=(std::uint64_t nv) noexcept { bits = nv; return *this; }

	constexpr bool test(size_t pos) const { return (*this)[pos]; }

	constexpr bool operator[](size_t pos) const { assert(pos < 64); return bits & (1ULL << pos); }
	constexpr bool all() const noexcept { return bits == UINT64_MAX; }
	constexpr bool any() const noexcept { return bits != 0; }
	constexpr bool none() const noexcept { return bits == 0; }

	constexpr size_t count() const noexcept
	{
		size_t        count     = 0;
		std::uint64_t temp_bits = bits;
		for (auto idx = std::countr_zero(temp_bits); idx < 64; idx = std::countr_zero(temp_bits))
		{
			count++;
			temp_bits ^= 1ULL << idx;
		}
		return count;
	}

	constexpr bitboard &operator&=(const bitboard &rhs) noexcept { bits &= rhs.bits; return *this; }
	constexpr bitboard &operator|=(const bitboard &rhs) noexcept { bits |= rhs.bits; return *this; }
	constexpr bitboard &operator^=(const bitboard &rhs) noexcept { bits ^= rhs.bits; return *this; }
	constexpr bitboard  operator~() const noexcept { return bitboard(~bits); }
	constexpr bitboard &operator~() noexcept { bits = ~bits; return *this; }

	constexpr bitboard &operator<<=(size_t n) noexcept { bits <<= n; return *this; }
	constexpr bitboard &operator>>=(size_t n) noexcept { bits >>= n; return *this; }
	constexpr bitboard  operator<<(size_t n) const noexcept { return bitboard(bits << n); }
	constexpr bitboard  operator>>(size_t n) const noexcept { return bitboard(bits >> n); }

	constexpr bitboard &set() noexcept { bits = UINT64_MAX; return *this; }
	constexpr bitboard &set(size_t pos, bool value = true) { assert(pos < 64); bits |= ((std::uint64_t) value) << pos; return *this; }

	constexpr bitboard &reset() noexcept { bits = 0; return *this; }
	constexpr bitboard &reset(size_t pos) { assert(pos < 64); bits &= ~(1ULL << pos); return *this; }

	constexpr bitboard &flip() noexcept { bits = ~bits; return *this; }
	constexpr bitboard &flip(size_t pos) { assert(pos < 64); bits ^= 1ULL << pos; return *this; }

	constexpr std::uint64_t to_ullong() const noexcept { return bits; }
	// clang-format on
};

constexpr bitboard operator&(const bitboard &a, const bitboard &b) noexcept { return bitboard(a.bits & b.bits); }
constexpr bitboard operator|(const bitboard &a, const bitboard &b) noexcept { return bitboard(a.bits | b.bits); }
constexpr bitboard operator^(const bitboard &a, const bitboard &b) noexcept { return bitboard(a.bits ^ b.bits); }

constexpr bitboard file_a = bitboard(0x0101010101010101);
constexpr bitboard rank_1 = bitboard(0xff);

struct threat_line
{
	bitboard line;
	// DirectionOffset direction;
	// if true then check line. if false then pin line
	bool     is_check;
};

struct piece_boards
{
	bitboard pawns;
	bitboard knights;
	bitboard bishops;
	bitboard rooks;
	bitboard queens;
	bitboard kings;
	bitboard all_pieces;
	// This bitboard shows all squares that are visible to this side's pieces.
	// The enemy king is x-rayed through this bitboard.
	bitboard visible;

	void calculate_combined() { all_pieces = pawns | knights | bishops | rooks | queens | kings; }

	bool operator==(const piece_boards &rhs)
	{
		return this->pawns == rhs.pawns && this->knights == rhs.knights && this->bishops == rhs.bishops
		       && this->rooks == rhs.rooks && this->queens == rhs.queens && this->kings == rhs.kings
		       && this->all_pieces == rhs.all_pieces && this->visible == rhs.visible;
	}

	bool operator!=(const piece_boards &rhs)
	{
		return this->pawns != rhs.pawns || this->knights != rhs.knights || this->bishops != rhs.bishops
		       || this->rooks != rhs.rooks || this->queens != rhs.queens || this->kings != rhs.kings
		       || this->all_pieces != rhs.all_pieces || this->visible != rhs.visible;
	}
};

struct list
{
	std::vector<bitboard> boards{};
	bitboard              combined;

	void calculate_combined()
	{
		combined = std::reduce(boards.begin(), boards.end(), bitboard(0), std::bit_or<bitboard>());
	}

	bool operator==(const list &rhs) { return this->boards == rhs.boards && this->combined == rhs.combined; }

	bool operator!=(const list &rhs) { return this->boards != rhs.boards || this->combined != rhs.combined; }
};

// The king can be attacked from a total of 16 sides: 8 cardinal directions, and the 8 knight moves.
// Knights can put the king in check, but they can't pin other pieces to the king.
// We don't care if there's more than one piece on a line, we just care that there
// is a piece that is on the line, which gives us a total of 8 lines that we care about.

struct threat_boards
{
	list checks;
	list pins;

	threat_boards()
	{
		checks.boards.reserve(16);
		pins.boards.reserve(8);
	}

	bool operator==(const threat_boards &rhs) { return this->checks == rhs.checks && this->pins == rhs.pins; }
	bool operator!=(const threat_boards &rhs) { return this->checks != rhs.checks || this->pins != rhs.pins; }
};

struct single_set
{
	piece_boards  pieces;
	threat_boards threats;

	bool operator==(const single_set &rhs) { return this->pieces == rhs.pieces && this->threats == rhs.threats; }
	bool operator!=(const single_set &rhs) { return this->pieces != rhs.pieces || this->threats != rhs.threats; }
};

struct full_set
{
	single_set white;
	single_set black;

	bool operator==(const full_set &rhs) { return this->white == rhs.white && this->black == rhs.black; }
	bool operator!=(const full_set &rhs) { return this->white != rhs.white || this->black != rhs.black; }
};

bitboard     generate_pawn_board(const Board &state, Color color);
bitboard     generate_knight_board(const Board &state, Color color);
bitboard     generate_bishop_board(const Board &state, Color color);
bitboard     generate_rook_board(const Board &state, Color color);
bitboard     generate_queen_board(const Board &state, Color color);
bitboard     generate_king_board(const Board &state, Color color);
piece_boards generate_piece_board(const Board &state, Color color);

// bitboard generate_pawn_visibility(const Board &state, const Piece &pawn);
// bitboard generate_knight_visibility(const Piece &knight);
// bitboard generate_bishop_visibility(const Board &state, const Piece &bishop);
// bitboard generate_rook_visibility(const Board &state, const Piece &rook);
// bitboard generate_queen_visibility(const Board &state, const Piece &queen);
// bitboard generate_king_visibility(const Board &state, const Piece &king);

bitboard generate_piece_visibility(const piece_set_t &piece_set, Color color, const full_set &old_boards);

threat_line generate_threat_line(const Piece    &piece,
                                 bitboard        all_pieces,
                                 bitboard        our_pieces,
                                 uint8_t         enemy_king_pos,
                                 DirectionOffset direction,
                                 size_t          max_steps);

threat_boards generate_threat_lines(const Board &state, Color color, const full_set &old_boards);

single_set generate_single_set(const Board &state, Color color);
full_set   generate_full_set(const Board &state);

std::string to_string(bitboard bb);

}