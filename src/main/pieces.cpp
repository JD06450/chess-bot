#include "pieces.hpp"

const piece_set_t::iterator piece_set_t::null_iterator = piece_set_t::iterator{};

char Piece::to_string() const
{
	char piece_string;
	switch (this->piece.type)
	{
	case PieceType::PAWN:
		piece_string = 'p';
		break;
	case PieceType::KNIGHT:
		piece_string = 'n';
		break;
	case PieceType::BISHOP:
		piece_string = 'b';
		break;
	case PieceType::ROOK:
		piece_string = 'r';
		break;
	case PieceType::QUEEN:
		piece_string = 'q';
		break;
	case PieceType::KING:
		piece_string = 'k';
		break;
	default:
		return '\0';
	}

	if (this->piece.color == WHITE) piece_string -= 32;

	return piece_string;
}

void Piece::promote_piece(PromotionOptions id)
{
	if (this->piece.type != PieceType::PAWN) return;
	switch (id)
	{
	case PromotionOptions::KNIGHT:
		this->set_piece(PieceType::KNIGHT);
		break;
	case PromotionOptions::BISHOP:
		this->set_piece(PieceType::BISHOP);
		break;
	case PromotionOptions::ROOK:
		this->set_piece(PieceType::ROOK);
		break;
	case PromotionOptions::QUEEN:
		this->set_piece(PieceType::QUEEN);
		break;
	}
}