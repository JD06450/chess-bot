#include "move.hpp"
#include "board.hpp"
#include "pieces.hpp"
#include <sstream>
#include <stdexcept>

Move::Move(uint16_t from, uint16_t to, uint16_t flags)
{
	this->_move.bits.from         = from;
	this->_move.bits.to           = to;
	this->_move.bits.flags = flags;
}

Move::Move(const std::string &from, const std::string &to, uint16_t flags)
{
	this->_move.bits.from = square_to_index(from);
	this->_move.bits.to = square_to_index(to);
	this->_move.bits.flags = flags;
}

char get_promotion_piece(uint16_t promotion_id, color_t c)
{
	switch (promotion_id)
	{
		case move_flags::KNIGHT: return c == WHITE ? 'N' : 'n';
		case move_flags::BISHOP: return c == WHITE ? 'B' : 'b';
		case move_flags::ROOK: return c == WHITE ? 'R' : 'r';
		case move_flags::QUEEN: return c == WHITE ? 'Q' : 'q';
	}
	return '#';
}

std::string Move::to_string(const Board &state, bool short_version) const
{
	piece_set_t::const_iterator piece = state.piece_board.at(this->get_from());
	// if (piece == nullptr) return "";
	std::ostringstream output;

	if (!this->is_capture() && !this->is_promotion() && !short_version)
	{
		if (this->_move.bits.flags == move_flags::KINGSIDE_CASTLE) return "O-O";
		if (this->_move.bits.flags == move_flags::QUEENSIDE_CASTLE) return "O-O-O";
	}

	color_t c = WHITE;
	if (piece != piece_set_t::null_iterator) c = piece->get_color();

	// if (piece == nullptr) output << "(Invalid): ";
	if (piece == piece_set_t::null_iterator && !short_version) throw std::runtime_error("Move generated for null piece?");
	else if (piece != piece_set_t::null_iterator && *piece != PieceType::PAWN && !short_version) output << piece->to_string();
	output << static_cast<char>(get_file_from_square(this->get_from()) + 'a') << static_cast<char>(get_rank_from_square(this->get_from()) + '1');
	if (this->is_capture() && !short_version) output << "x";
	output << static_cast<char>(get_file_from_square(this->get_to()) + 'a') << static_cast<char>(get_rank_from_square(this->get_to()) + '1');
	if (this->is_promotion() && !short_version) output << "=" << get_promotion_piece(this->get_special(), c);
	else if (this->is_promotion()) output << get_promotion_piece(this->get_special(), BLACK);

	return output.str();
}