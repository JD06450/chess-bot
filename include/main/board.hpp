#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <stack>
#include <string>

// #include "arraylist.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "pieces.hpp"

class Board
{
public:
	struct CastlingRights
	{
		bool kingside  = true;
		bool queenside = true;
	};
	struct IrreversableState
	{
		Board::CastlingRights white_castling_rights;
		Board::CastlingRights black_castling_rights;
		uint16_t              fifty_move_clock  = 0;
		int16_t               en_passant_target = -1;
		Piece                 captured_piece;
	};

	std::array<piece_set_t::iterator, 64> pieces{};
	std::list<Move>         moves;
private:
	std::stack<IrreversableState> history;
public:
	bitboard::full_set bitboards;

	piece_set_t white_pieces;
	piece_set_t black_pieces;
private:
	uint32_t halfmove          = 0;
	uint16_t fifty_move_clock  = 0;
	int16_t  en_passant_target = -1;

	CastlingRights white_castling_rights;
	CastlingRights black_castling_rights;

	bool _in_check = false;
public:
	Board() {}
	Board(const Board &b);
	Board(const Board &&b);

	static std::optional<Board> from_fen(const std::string &fen_string);

	Board &operator=(const Board &b);
	Board &operator=(const Board &&b);

	std::string    to_string() const;
	inline Color   turn_to_move() const { return halfmove % 2 == 0 ? Color::WHITE : Color::BLACK; }
	bool is_in_check() const { return _in_check; }

	inline unsigned int   get_halfmoves() const { return halfmove; }
	inline uint16_t       get_last_capture_or_pawn_push() const { return fifty_move_clock; }
	inline int16_t        get_en_passant_target() const { return en_passant_target; }
	inline bool           can_en_passant() const { return en_passant_target != -1; }
	inline CastlingRights get_white_castling_rights() const { return white_castling_rights; }
	inline CastlingRights get_black_castling_rights() const { return black_castling_rights; }
	inline CastlingRights get_castling_rights(Color c) const
	{
		return c == Color::WHITE ? white_castling_rights : black_castling_rights;
	}
	inline const bitboard::full_set &get_bitboards() const { return bitboards; };

	std::array<piece_set_t::iterator, 64>             &get_pieces();
	const std::array<piece_set_t::const_iterator, 64> &get_pieces() const;

	void add_piece(Piece piece);
	void make_move(Move m);
	void unmake_move();
	void update_bitboards() { this->bitboards = bitboard::generate_bitboard_full_set(*this); };

	Board simulate_move(Move m) const;

private:
	void _setup_piece_iterators();

	void _move_piece(uint16_t from, uint16_t to, piece_set_t::iterator &moved_piece, bitboard::single_set &bb_set);
	void _delete_captured_piece(piece_set_t::iterator &piece);

	void _handle_castling_rights(Move &m, piece_set_t::const_iterator from_piece, piece_set_t::const_iterator to_piece);
	void _handle_castling(Move &m, bool kingside);
	void _handle_undo_castling(Move &m, bool kingside);

	void _handle_promotion(Move m, Color current_color, piece_set_t::iterator from_piece, bitboard::single_set &set);
	void _handle_undo_promotion(Move m, Color current_color, piece_set_t::iterator from_piece, bitboard::single_set &set);
};

unsigned int square_to_index(const std::string &square);