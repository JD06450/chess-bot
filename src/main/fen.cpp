#include "fen.hpp"
#include <array>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <optional>
#include <utility>

#include "board.hpp"
#include "pieces.hpp"

Piece parse_piece(char p, uint8_t idx)
{
	Piece piece;

	piece.position(idx);

	if (p >= 'a')
	{
		piece.set_color(Color::BLACK);
		p -= 'a' - 'A';
	}
	else
	{
		piece.set_color(Color::WHITE);
	}

	switch (p)
	{
	case 'P':
		piece.set_piece(PieceType::PAWN);
		break;
	case 'N':
		piece.set_piece(PieceType::KNIGHT);
		break;
	case 'B':
		piece.set_piece(PieceType::BISHOP);
		break;
	case 'R':
		piece.set_piece(PieceType::ROOK);
		break;
	case 'Q':
		piece.set_piece(PieceType::QUEEN);
		break;
	case 'K':
		piece.set_piece(PieceType::KING);
		break;
	default:
		return Piece();
	}

	return piece;
}

bool add_pieces_to_board(Board& board, std::string pieces_str)
{
	std::array<std::string, 8> pieces;

	size_t prev_offset = 0, offset = 0;

	for (int i = 7; i >= 0; i--)
	{
		offset = pieces_str.find('/', offset);
		if (i == 0) offset = pieces_str.length();
		if (offset == std::string::npos) return false;
		pieces.at(i) = pieces_str.substr(prev_offset, offset - prev_offset);
		prev_offset = ++offset;
	}

	for (int rank = 0; rank < pieces.size(); rank++)
	{
		const std::string& file_str = pieces.at(rank);
		if (file_str.size() > 8) return false;
		int offset = 0;
		for (int file = 0; file < file_str.size(); file++)
		{
			char p = file_str.at(file);
			if (std::isdigit(p))
			{
				offset += p - '0' - 1;
				continue;
			}
			Piece new_piece = parse_piece(p, rank * 8 + file + offset);
			if (new_piece.is_none()) return false;
			board.add_piece(new_piece);
		}
	}

	return true;
}

std::pair<Board::CastlingRights, Board::CastlingRights> parse_castling_rights(const std::string &rights_string)
{
	std::pair<Board::CastlingRights, Board::CastlingRights> rights{{false, false}, {false, false}};

	if (rights_string == "-") return rights;

	if (rights_string.contains('K')) rights.first.kingside = true;
	if (rights_string.contains('Q')) rights.first.queenside = true;
	if (rights_string.contains('k')) rights.second.kingside = true;
	if (rights_string.contains('q')) rights.second.queenside = true;

	return rights;
}

std::optional<Board> Board::from_fen(const std::string &fen_string)
{
	std::array<std::string, 6> fields;
	
	size_t prev_offset = 0, offset = 0;

	for (size_t i = 0; i < fields.size(); i++)
	{
		offset = fen_string.find(' ', offset);
		if (i == fields.size() - 1) offset = fen_string.length();
		if (offset == std::string::npos) return std::nullopt;
		fields.at(i) = fen_string.substr(prev_offset, offset - prev_offset);
		prev_offset = ++offset;
	}

	Board board{};
	if (!add_pieces_to_board(board, fields[0])) return std::nullopt;

	int turn_number = std::strtol(fields[5].c_str(), 0, 10);
	int turn_offset = 0;
	if (fields[1][0] == 'b') turn_offset = 1;
	else if (fields[1][0] != 'w') return std::nullopt;
	if (turn_number == 0) return std::nullopt;
	
	board.halfmove = (turn_number - 1) * 2 + turn_offset;

	board.fifty_move_clock = std::strtol(fields[4].c_str(), 0, 10);
	if (board.fifty_move_clock == 0 && fields[4][0] != '0') return std::nullopt;

	auto castling_rights = parse_castling_rights(fields[2]);
	board.white_castling_rights = castling_rights.first;
	board.black_castling_rights = castling_rights.second;

	if (fields[3] != "-") board.en_passant_target = square_to_index(fields[3]);

	return board;
}

std::string generate_fen_string(const Board &board)
{
	return "";
}