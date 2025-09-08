#pragma once

#include <array>
#include <cstdint>
#include <string>

enum class DirectionOffset
{
	NONE = 0,

	UP    = 8,
	DOWN  = -8,
	LEFT  = -1,
	RIGHT = 1,

	UP_LEFT    = 7,
	UP_RIGHT   = 9,
	DOWN_LEFT  = -9,
	DOWN_RIGHT = -7
};

inline constexpr DirectionOffset flip_direction(DirectionOffset d)
{
	switch (d)
	{
	case DirectionOffset::UP:         return DirectionOffset::DOWN;
	case DirectionOffset::DOWN:       return DirectionOffset::UP;
	case DirectionOffset::LEFT:       return DirectionOffset::RIGHT;
	case DirectionOffset::RIGHT:      return DirectionOffset::LEFT;
	case DirectionOffset::UP_LEFT:    return DirectionOffset::DOWN_RIGHT;
	case DirectionOffset::UP_RIGHT:   return DirectionOffset::DOWN_LEFT;
	case DirectionOffset::DOWN_LEFT:  return DirectionOffset::UP_RIGHT;
	case DirectionOffset::DOWN_RIGHT: return DirectionOffset::UP_LEFT;
	default:                          return DirectionOffset::NONE;
	}
}

inline constexpr DirectionOffset get_horizontal(DirectionOffset d)
{
	switch (d)
	{
	case DirectionOffset::UP:         return DirectionOffset::NONE;
	case DirectionOffset::DOWN:       return DirectionOffset::NONE;
	case DirectionOffset::LEFT:       return DirectionOffset::LEFT;
	case DirectionOffset::RIGHT:      return DirectionOffset::RIGHT;
	case DirectionOffset::UP_LEFT:    return DirectionOffset::LEFT;
	case DirectionOffset::UP_RIGHT:   return DirectionOffset::RIGHT;
	case DirectionOffset::DOWN_LEFT:  return DirectionOffset::LEFT;
	case DirectionOffset::DOWN_RIGHT: return DirectionOffset::RIGHT;
	default:                          return DirectionOffset::NONE;
	}
}

inline constexpr DirectionOffset get_vertical(DirectionOffset d)
{
	switch (d)
	{
	case DirectionOffset::UP:         return DirectionOffset::UP;
	case DirectionOffset::DOWN:       return DirectionOffset::DOWN;
	case DirectionOffset::LEFT:       return DirectionOffset::NONE;
	case DirectionOffset::RIGHT:      return DirectionOffset::NONE;
	case DirectionOffset::UP_LEFT:    return DirectionOffset::UP;
	case DirectionOffset::UP_RIGHT:   return DirectionOffset::UP;
	case DirectionOffset::DOWN_LEFT:  return DirectionOffset::DOWN;
	case DirectionOffset::DOWN_RIGHT: return DirectionOffset::DOWN;
	default:                          return DirectionOffset::NONE;
	}
}

enum class MoveNotation
{
	ALGEBRAIC,
	LONG_ALGEBRAIC
};

inline constexpr uint32_t get_file_from_square(uint32_t square) { return square % 8; }
inline constexpr uint32_t get_rank_from_square(uint32_t square) { return square / 8; }
// Using an unsigned int, so no need to check if less than zero.
inline constexpr bool     inside_board(uint32_t square) { return square < 64; }

constexpr std::array<DirectionOffset, 8> DIRECTION_OFFSETS = {
	DirectionOffset::UP,      DirectionOffset::DOWN,     DirectionOffset::LEFT,      DirectionOffset::RIGHT,
	DirectionOffset::UP_LEFT, DirectionOffset::UP_RIGHT, DirectionOffset::DOWN_LEFT, DirectionOffset::DOWN_RIGHT
};

constexpr std::array<int, 8> KNIGHT_DIRECTION_OFFSETS = { 6, 15, 17, 10, -6, -15, -17, -10 };

class Board;

enum class PromotionOptions : uint8_t
{
	KNIGHT      = 0,
	BISHOP      = 1,
	ROOK        = 2,
	QUEEN       = 3,
	MAX_OPTIONS = 4
};

namespace move_flags
{

// Piecewise definitions

constexpr uint8_t KNIGHT = (uint8_t) PromotionOptions::KNIGHT;
constexpr uint8_t BISHOP = (uint8_t) PromotionOptions::BISHOP;
constexpr uint8_t ROOK   = (uint8_t) PromotionOptions::ROOK;
constexpr uint8_t QUEEN  = (uint8_t) PromotionOptions::QUEEN;

constexpr uint8_t CAPTURE   = 0b0100;
constexpr uint8_t PROMOTION = 0b1000;

// Exact values

constexpr uint8_t QUIET_MOVE       = 0b0000;
constexpr uint8_t DOUBLE_PAWN_PUSH = 0b0001;
constexpr uint8_t KINGSIDE_CASTLE  = 0b0010;
constexpr uint8_t QUEENSIDE_CASTLE = 0b0011;

constexpr uint8_t NORMAL_CAPTURE = 0b0100;
constexpr uint8_t EN_PASSANT     = 0b0101;

constexpr uint8_t KNIGHT_PROMOTION = PROMOTION | KNIGHT;
constexpr uint8_t BISHOP_PROMOTION = PROMOTION | BISHOP;
constexpr uint8_t ROOK_PROMOTION   = PROMOTION | ROOK;
constexpr uint8_t QUEEN_PROMOTION  = PROMOTION | QUEEN;

constexpr uint8_t KNIGHT_CAPTURE_PROMOTION = PROMOTION | CAPTURE | KNIGHT;
constexpr uint8_t BISHOP_CAPTURE_PROMOTION = PROMOTION | CAPTURE | BISHOP;
constexpr uint8_t ROOK_CAPTURE_PROMOTION   = PROMOTION | CAPTURE | ROOK;
constexpr uint8_t QUEEN_CAPTURE_PROMOTION  = PROMOTION | CAPTURE | QUEEN;

}

class Move
{
public:
	// TODO: Refactor special flags to be integer flags alongside promotion and castling,
	// as those actions heavily influence what the special flags mean
	// enum class SpecialFlag : uint8_t
	// {
	// 	QUIET_MOVE       = 0b00,
	// 	DOUBLE_PAWN_PUSH = 0b01,
	// 	KINGSIDE_CASTLE  = 0b10,
	// 	QUEENSIDE_CASTLE = 0b11,

	// 	NORMAL_CAPTURE = 0b00,
	// 	EN_PASSANT     = 0b01,

	// 	KNIGHT_PROMOTION = 0b00,
	// 	BISHOP_PROMOTION = 0b01,
	// 	ROOK_PROMOTION   = 0b10,
	// 	QUEEN_PROMOTION  = 0b11,
	// };

private:
	union
	{
		struct
		{
			uint16_t from  : 6;
			uint16_t to    : 6;
			// // Flag must be a single value from the `SpecialFlag` enum
			// SpecialFlag special      : 2;
			// bool        is_capture   : 1;
			// bool        is_promotion : 1;
			uint16_t flags : 4;
		} bits;
		uint16_t raw;
	} _move;

public:
	Move() noexcept;
	Move(const Move &m) = default;
	Move(uint16_t from, uint16_t to, uint16_t flags = move_flags::QUIET_MOVE);
	Move(const std::string &from, const std::string &to, uint16_t flags = move_flags::QUIET_MOVE);

	uint16_t get_from() const { return _move.bits.from; }
	uint16_t get_to() const { return _move.bits.to; }
	// SpecialFlag get_special() const { return _move.bits.special; }
	uint16_t get_flags() const { return _move.bits.flags; }
	uint16_t get_special() const { return _move.bits.flags & 0b11; }
	bool     is_promotion() const { return _move.bits.flags & move_flags::PROMOTION; }
	bool     is_capture() const { return _move.bits.flags & move_flags::CAPTURE; }

	bool operator==(const Move &rhs) { return this->_move.raw == rhs._move.raw; }
	bool operator!=(const Move &rhs) { return this->_move.raw != rhs._move.raw; }

	void set_from(uint16_t from) { _move.bits.from = from; }
	void set_to(uint16_t to) { _move.bits.to = to; }
	// void set_special(SpecialFlag special) { _move.bits.special = special; }
	void set_promotion(bool p)
	{
		if (p) _move.bits.flags |= move_flags::PROMOTION;
		else _move.bits.flags &= 0b1111 ^ move_flags::PROMOTION;
	}
	void set_capture(bool c)
	{
		if (c) _move.bits.flags |= move_flags::CAPTURE;
		else _move.bits.flags &= 0b1111 ^ move_flags::CAPTURE;
	}

	bool empty() const { return _move.raw == 0; }
	bool is_none() const { return this->empty(); }

	std::string to_string(const Board &state, bool short_version = false) const;
};