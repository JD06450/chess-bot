#pragma once
#include "move.hpp"
#include <cassert>
#include <cstdint>
#include <list>

enum class PieceType : uint8_t
{
	NONE     = 0,
	PAWN     = 1,
	KNIGHT   = 2,
	BISHOP   = 3,
	ROOK     = 4,
	QUEEN    = 5,
	KING     = 6,
	MAX_TYPE = 7
};

typedef uint8_t   color_t;
constexpr color_t WHITE = 0;
constexpr color_t BLACK = 1;

inline constexpr color_t invert_color(color_t c) { return !c; }

class Piece
{
private:
	struct
	{
		PieceType type  : 3;
		color_t   color : 1;
	} piece;
	uint8_t _position = 0;

public:
	constexpr uint8_t position() const noexcept { return _position; }

	constexpr uint8_t position(uint8_t new_pos)
	{
		assert(new_pos < 64);
		_position = new_pos;
		return _position;
	}

	constexpr Piece() noexcept : piece(PieceType::NONE) {}
	constexpr Piece(PieceType type, color_t color = WHITE) noexcept
	{
		this->piece.type  = type;
		this->piece.color = color;
		this->_position   = 0;
	}

	constexpr Piece(PieceType type, color_t color, uint8_t pos)
	{
		this->piece.type  = type;
		this->piece.color = color;
		assert(pos < 64);
		this->_position = pos;
	}

	constexpr Piece(const Piece &p)            = default;
	constexpr Piece &operator=(const Piece &p) = default;
	// Piece &operator=(const Piece &&p) = default;

	constexpr      operator bool() const { return !this->empty(); }
	constexpr bool operator==(const PieceType &p) const noexcept { return this->piece.type == p; }
	constexpr bool operator!=(const PieceType &p) const noexcept { return this->piece.type != p; }
	constexpr bool operator==(const color_t &c) const noexcept { return this->piece.color == c; }
	constexpr bool operator!=(const color_t &c) const noexcept { return this->piece.color != c; }

	constexpr bool operator==(const Piece &p) const noexcept
	{
		return piece.type == p.piece.type && piece.color == p.piece.color && _position == p._position;
	}

	constexpr bool is_same(const Piece &p) const noexcept
	{
		return this->piece.type == p.get_type() && this->piece.color == p.get_color();
	}

	constexpr bool empty() const noexcept { return piece.type == PieceType::NONE; }
	constexpr bool is_none() const noexcept { return empty(); }

	constexpr void clear() noexcept
	{
		this->set_color(WHITE);
		this->set_piece(PieceType::NONE);
	}

	constexpr color_t   get_color() const noexcept { return this->piece.color; }
	constexpr PieceType get_type() const noexcept { return this->piece.type; }
	constexpr void      set_color(color_t c) noexcept { this->piece.color = c; }
	constexpr void      set_piece(PieceType p) noexcept { this->piece.type = p; }

	void promote_piece(PromotionOptions id);

	char to_string() const;
};

struct piece_set_t
{
	typedef std::list<Piece> PieceList;

	typedef PieceList::iterator       iterator;
	typedef PieceList::const_iterator const_iterator;

	static const iterator null_iterator;

	// We don't really need to store multiple kings,
	// but this does make things easier.
	typedef PieceList KingList;
	typedef PieceList QueenList;
	typedef PieceList RookList;
	typedef PieceList BishopList;
	typedef PieceList KnightList;
	typedef PieceList PawnList;

	KingList   kings;
	QueenList  queens;
	RookList   rooks;
	BishopList bishops;
	KnightList knights;
	PawnList   pawns;
};