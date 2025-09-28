#include "board.hpp"
#include "bitboard.hpp"
#include "move.hpp"
#include "move_generation.hpp"
#include "pieces.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <stdexcept>

void Board::_setup_piece_iterators()
{
	// pieces[this->pieces[WHITE].kings.front().position()] = this->pieces[WHITE].kings.begin();
	for (auto king = this->pieces[WHITE].kings.begin(); king != this->pieces[WHITE].kings.end(); king++)
		piece_board[king->position()] = king;

	for (auto queen = this->pieces[WHITE].queens.begin(); queen != this->pieces[WHITE].queens.end(); queen++)
		piece_board[queen->position()] = queen;
	for (auto rook = this->pieces[WHITE].rooks.begin(); rook != this->pieces[WHITE].rooks.end(); rook++)
		piece_board[rook->position()] = rook;
	for (auto bishop = this->pieces[WHITE].bishops.begin(); bishop != this->pieces[WHITE].bishops.end(); bishop++)
		piece_board[bishop->position()] = bishop;
	for (auto knight = this->pieces[WHITE].knights.begin(); knight != this->pieces[WHITE].knights.end(); knight++)
		piece_board[knight->position()] = knight;
	for (auto pawn = this->pieces[WHITE].pawns.begin(); pawn != this->pieces[WHITE].pawns.end(); pawn++)
		piece_board[pawn->position()] = pawn;

	// pieces[this->pieces[BLACK].kings.front().position()] = this->pieces[BLACK].kings.begin();
	for (auto king = this->pieces[BLACK].kings.begin(); king != this->pieces[BLACK].kings.end(); king++)
		piece_board[king->position()] = king;

	for (auto queen = this->pieces[BLACK].queens.begin(); queen != this->pieces[BLACK].queens.end(); queen++)
		piece_board[queen->position()] = queen;
	for (auto rook = this->pieces[BLACK].rooks.begin(); rook != this->pieces[BLACK].rooks.end(); rook++)
		piece_board[rook->position()] = rook;
	for (auto bishop = this->pieces[BLACK].bishops.begin(); bishop != this->pieces[BLACK].bishops.end(); bishop++)
		piece_board[bishop->position()] = bishop;
	for (auto knight = this->pieces[BLACK].knights.begin(); knight != this->pieces[BLACK].knights.end(); knight++)
		piece_board[knight->position()] = knight;
	for (auto pawn = this->pieces[BLACK].pawns.begin(); pawn != this->pieces[BLACK].pawns.end(); pawn++)
		piece_board[pawn->position()] = pawn;
}

Board::Board(const Board &b) :
    // pieces_setup(false),
    piece_board({}), pieces(b.pieces), rights(b.rights), moves(b.moves), halfmove(b.halfmove),
    fifty_move_clock(b.fifty_move_clock), en_passant_target(b.en_passant_target), bitboards(b.bitboards),
    history(b.history)
{
	this->_setup_piece_iterators();
}

Board::Board(const Board &&b) :
    // pieces_setup(false),
    piece_board(std::move(b.piece_board)), pieces(std::move(b.pieces)), rights(std::move(b.rights)),
    moves(std::move(b.moves)), halfmove(b.halfmove), fifty_move_clock(b.fifty_move_clock),
    en_passant_target(b.en_passant_target), bitboards(std::move(b.bitboards)), history(std::move(b.history))
{
}

Board &Board::operator=(const Board &b)
{
	if (this == &b) return *this;

	this->pieces = b.pieces;

	this->bitboards = b.bitboards;

	this->halfmove          = b.halfmove;
	this->fifty_move_clock  = b.fifty_move_clock;
	this->en_passant_target = b.en_passant_target;

	this->moves   = b.moves;
	this->history = b.history;

	this->rights = b.rights;

	this->_setup_piece_iterators();

	return *this;
}

Board &Board::operator=(const Board &&b)
{
	if (this == &b) return *this;

	this->pieces = std::move(b.pieces);

	this->piece_board = std::move(b.piece_board);
	this->bitboards   = std::move(b.bitboards);

	this->halfmove          = b.halfmove;
	this->fifty_move_clock  = b.fifty_move_clock;
	this->en_passant_target = b.en_passant_target;

	this->moves   = std::move(b.moves);
	this->history = std::move(b.history);

	this->rights = std::move(b.rights);

	return *this;
}

std::string Board::to_string() const
{
	std::ostringstream boardStr;
	// From white's perspective, the ranks go from bottom-to-top.
	// Appending rows to the stream will go from top-to-bottom,
	// so the order needs to be reversed here.
	for (int rank = 7; rank >= 0; rank--)
	{
		for (int file = 0; file < 8; file++)
		{
			bool                        is_dark_square  = (file + rank) % 2;
			piece_set_t::const_iterator piece_at_square = this->piece_board.at(rank * 8 + file);

			if (piece_at_square != piece_set_t::const_iterator{}) boardStr << piece_at_square->to_string() << ' ';
			else boardStr << (is_dark_square ? '.' : '#') << ' ';
		}
		boardStr << '\n';
	}
	return boardStr.str();
}

void Board::add_piece(Piece piece)
{
	color_t color = piece.get_color();
	// These lines are really long. Should maybe look for a better way to do this
	switch (piece.get_type())
	{
	case PieceType::PAWN:
		this->piece_board.at(piece.position()) = this->pieces[color].pawns.insert(this->pieces[color].pawns.end(),
		                                                                          piece);
		break;
	case PieceType::KNIGHT:
		this->piece_board.at(piece.position()) = this->pieces[color].knights.insert(this->pieces[color].knights.end(),
		                                                                            piece);
		break;
	case PieceType::BISHOP:
		this->piece_board.at(piece.position()) = this->pieces[color].bishops.insert(this->pieces[color].bishops.end(),
		                                                                            piece);
		break;
	case PieceType::ROOK:
		this->piece_board.at(piece.position()) = this->pieces[color].rooks.insert(this->pieces[color].rooks.end(),
		                                                                          piece);
		break;
	case PieceType::QUEEN:
		this->piece_board.at(piece.position()) = this->pieces[color].queens.insert(this->pieces[color].queens.end(),
		                                                                           piece);
		break;
	case PieceType::KING:
		this->piece_board.at(piece.position()) = this->pieces[color].kings.insert(this->pieces[color].kings.end(),
		                                                                          piece);
		break;
	default: this->piece_board.at(piece.position()) = piece_set_t::iterator{};
	}
}

inline bitboard::bitboard &get_piece_bitboard(bitboard::single_set &set, Piece p)
{
	switch (p.get_type())
	{
	case PieceType::PAWN:   return set.pieces.pawns;
	case PieceType::KNIGHT: return set.pieces.knights;
	case PieceType::BISHOP: return set.pieces.bishops;
	case PieceType::ROOK:   return set.pieces.rooks;
	case PieceType::QUEEN:  return set.pieces.queens;
	case PieceType::KING:   return set.pieces.kings;
	// Have to return something, even though I really shouldn't have to here.
	default:                return set.pieces.all_pieces;
	}
}

bool in_check(const Board *state)
{
	color_t                     c         = state->turn_to_move();
	const bitboard::single_set &set       = state->bitboards[c];
	const bitboard::single_set &other_set = state->bitboards[invert_color(c)];

	return (set.pieces.kings & other_set.pieces.visible).any();
}

void Board::_move_piece(uint16_t from, uint16_t to, piece_set_t::iterator &moved_piece, bitboard::single_set &bb_set)
{
	get_piece_bitboard(bb_set, *moved_piece).reset(from).set(to);
	bb_set.pieces.all_pieces.reset(from).set(to);

	assert(to < 64);
	moved_piece->position(to);
	this->piece_board.at(to) = moved_piece;
	moved_piece              = piece_set_t::null_iterator;
}

#pragma region CASTLING

constexpr int16_t KINGSIDE_CASTLE_PIECE_OFFSET  = ((int16_t) DirectionOffset::RIGHT) * 3;
constexpr int16_t QUEENSIDE_CASTLE_PIECE_OFFSET = ((int16_t) DirectionOffset::LEFT) * 4;
constexpr int16_t KINGSIDE_CASTLE_END_OFFSET    = (int16_t) DirectionOffset::LEFT;
constexpr int16_t QUEENSIDE_CASTLE_END_OFFSET   = (int16_t) DirectionOffset::RIGHT;

struct rook_positions_t
{
	uint8_t queenside;
	uint8_t kingside;
};

void Board::_handle_castling_rights(Move                       &m,
                                    piece_set_t::const_iterator from_piece,
                                    piece_set_t::const_iterator target_piece)
{
	static constexpr std::array<rook_positions_t, 2> test_positions = { 0, 7, 56, 63 };

	color_t                 c                    = this->turn_to_move();
	CastlingRights         &our_rights           = this->rights[c];
	CastlingRights         &enemy_rights         = this->rights[invert_color(c)];
	const rook_positions_t &our_test_positions   = test_positions[c];
	const rook_positions_t &enemy_test_positions = test_positions[invert_color(c)];

	bool rook_captured = target_piece != piece_set_t::null_iterator && *target_piece == PieceType::ROOK;
	bool rook_moved    = *from_piece == PieceType::ROOK;

	if (rook_moved)
	{
		if (from_piece->position() == our_test_positions.queenside) our_rights.queenside = false;
		else if (from_piece->position() == our_test_positions.kingside) our_rights.kingside = false;
	}
	else if (rook_captured)
	{
		if (target_piece->position() == enemy_test_positions.queenside) enemy_rights.queenside = false;
		else if (target_piece->position() == enemy_test_positions.kingside) enemy_rights.kingside = false;
	}
	else if (*from_piece == PieceType::KING)
	{
		our_rights.kingside  = false;
		our_rights.queenside = false;
	}
}

void Board::_handle_castling(Move &m, bool kingside)
{
	const int16_t   rook_offset     = kingside ? KINGSIDE_CASTLE_PIECE_OFFSET : QUEENSIDE_CASTLE_PIECE_OFFSET;
	const int16_t   rook_end_offset = kingside ? KINGSIDE_CASTLE_END_OFFSET : QUEENSIDE_CASTLE_END_OFFSET;
	color_t         c               = this->turn_to_move();
	CastlingRights &rights          = this->rights[c];

	if (kingside && !rights.kingside) throw std::logic_error("Invalid kingside castle.");
	if (!kingside && !rights.queenside) throw std::logic_error("Invalid queenside castle.");

	piece_set_t::iterator &rook = this->piece_board.at(m.get_from() + rook_offset);

	if (rook == piece_set_t::null_iterator || *rook != PieceType::ROOK)
		throw std::logic_error("Invalid castle. Cannot find rook.");

	const int16_t rook_end_position = m.get_to() + rook_end_offset;
	const int16_t king_end_position = m.get_to();

	if (this->piece_board.at(rook_end_position) != piece_set_t::null_iterator
	    && this->piece_board.at(king_end_position) != piece_set_t::null_iterator)
		throw std::logic_error("Invalid castle. Squares not empty");

	bitboard::single_set &set = this->bitboards[c];
	_move_piece(rook->position(), rook_end_position, rook, set);
}

void Board::_handle_undo_castling(Move &m, bool kingside)
{
	const int16_t rook_offset     = kingside ? KINGSIDE_CASTLE_END_OFFSET : QUEENSIDE_CASTLE_END_OFFSET;
	const int16_t rook_end_offset = kingside ? KINGSIDE_CASTLE_PIECE_OFFSET : QUEENSIDE_CASTLE_PIECE_OFFSET;
	color_t       c               = this->turn_to_move();

	piece_set_t::iterator &rook = this->piece_board.at(m.get_to() + rook_offset);
	assert(rook != piece_set_t::null_iterator && rook->get_type() == PieceType::ROOK && "Cannot find rook to uncastle");

	int16_t rook_end_position = m.get_from() + rook_end_offset;

	bitboard::single_set &set = this->bitboards[c];
	_move_piece(rook->position(), rook_end_position, rook, set);
}

void Board::_delete_captured_piece(piece_set_t::iterator &captured_piece)
{
	assert(captured_piece != piece_set_t::null_iterator);
	uint8_t               position = captured_piece->position();
	color_t               c        = captured_piece->get_color();
	piece_set_t          &set      = this->pieces[c];
	bitboard::single_set &bb_set   = this->bitboards[c];

	switch (captured_piece->get_type())
	{
	case PieceType::PAWN:
		if (std::find(set.pawns.begin(), set.pawns.end(), *captured_piece) == set.pawns.end())
			throw std::runtime_error("Deleted pawn not in list");
		set.pawns.erase(captured_piece);
		break;
	case PieceType::KNIGHT:
		if (std::find(set.knights.begin(), set.knights.end(), *captured_piece) == set.knights.end())
			throw std::runtime_error("Deleted knight not in list");
		set.knights.erase(captured_piece);
		break;
	case PieceType::BISHOP:
		if (std::find(set.bishops.begin(), set.bishops.end(), *captured_piece) == set.bishops.end())
			throw std::runtime_error("Deleted bishop not in list");
		set.bishops.erase(captured_piece);
		break;
	case PieceType::ROOK:
		if (std::find(set.rooks.begin(), set.rooks.end(), *captured_piece) == set.rooks.end())
			throw std::runtime_error("Deleted rook not in list");
		set.rooks.erase(captured_piece);
		break;
	case PieceType::QUEEN:
		if (std::find(set.queens.begin(), set.queens.end(), *captured_piece) == set.queens.end())
			throw std::runtime_error("Deleted queen not in list");
		set.queens.erase(captured_piece);
		break;
	case PieceType::KING: throw std::invalid_argument("King cannot be captured. Invalid piece capture!"); break;
	default:              return;
	}

	get_piece_bitboard(bb_set, *captured_piece).reset(captured_piece->position());
	bb_set.pieces.all_pieces.reset(captured_piece->position());
	captured_piece = piece_set_t::null_iterator;
}

#pragma endregion CASTLING

#pragma region PROMOTIONS

void Board::_handle_promotion(Move                  m,
                              color_t               current_color,
                              piece_set_t::iterator from_piece,
                              bitboard::single_set &bb_set)
{
	from_piece->promote_piece(static_cast<PromotionOptions>(m.get_special()));
	bb_set.pieces.pawns.reset(m.get_from());

	piece_set_t &piece_set = this->pieces[current_color];

	switch (m.get_special())
	{
	case move_flags::KNIGHT: piece_set.knights.splice(piece_set.knights.end(), piece_set.pawns, from_piece); break;
	case move_flags::BISHOP: piece_set.bishops.splice(piece_set.bishops.end(), piece_set.pawns, from_piece); break;
	case move_flags::ROOK:   piece_set.rooks.splice(piece_set.rooks.end(), piece_set.pawns, from_piece); break;
	case move_flags::QUEEN:  piece_set.queens.splice(piece_set.queens.end(), piece_set.pawns, from_piece); break;
	}
}

void Board::_handle_undo_promotion(Move                  m,
                                   color_t               current_color,
                                   piece_set_t::iterator moved_piece,
                                   bitboard::single_set &bb_set)
{
	piece_set_t &piece_set = this->pieces[current_color];
	get_piece_bitboard(bb_set, *moved_piece).reset(m.get_to());
	moved_piece->set_piece(PieceType::PAWN);
	bb_set.pieces.pawns.set(m.get_from());

	switch (m.get_special())
	{
	case move_flags::KNIGHT: piece_set.pawns.splice(piece_set.pawns.end(), piece_set.knights, moved_piece); break;
	case move_flags::BISHOP: piece_set.pawns.splice(piece_set.pawns.end(), piece_set.bishops, moved_piece); break;
	case move_flags::ROOK:   piece_set.pawns.splice(piece_set.pawns.end(), piece_set.rooks, moved_piece); break;
	case move_flags::QUEEN:  piece_set.pawns.splice(piece_set.pawns.end(), piece_set.queens, moved_piece); break;
	}
}

#pragma endregion PROMOTIONS

#pragma region MOVE_PROCESSING

void Board::make_move(Move m)
{
	color_t current_color = this->turn_to_move();
	color_t other_color   = invert_color(current_color);
	bool    is_en_passant = m.get_flags() == move_flags::EN_PASSANT;
	bool is_castle_move = m.get_flags() == move_flags::KINGSIDE_CASTLE || m.get_flags() == move_flags::QUEENSIDE_CASTLE;

	DirectionOffset offset_from_to = PAWN_MOVE_OFFSETS[other_color];

	uint16_t from_square = m.get_from(), to_square = m.get_to(),
	         en_passant_square = to_square + (int16_t) offset_from_to,
	         target_square = is_en_passant ? en_passant_square : to_square, flags = m.get_flags();

	piece_set_t::iterator &from_piece   = this->piece_board.at(from_square);
	piece_set_t::iterator &target_piece = this->piece_board.at(target_square);
	piece_set_t::iterator &to_piece     = this->piece_board.at(to_square);
	bitboard::single_set  &set          = this->bitboards[current_color];
	bitboard::single_set  &other_set    = this->bitboards[other_color];

	if (from_piece == piece_set_t::null_iterator) throw std::invalid_argument("'From' target must be a valid piece.");

	IrreversableState old_state;
	old_state.rights            = this->rights;
	old_state.en_passant_target = this->en_passant_target;
	old_state.fifty_move_clock  = this->fifty_move_clock;
	if (target_piece != piece_set_t::null_iterator) old_state.captured_piece = *target_piece;

	this->en_passant_target = -1;

	if (flags == move_flags::DOUBLE_PAWN_PUSH) this->en_passant_target = en_passant_square;
	else if (is_castle_move) this->_handle_castling(m, m.get_flags() == move_flags::KINGSIDE_CASTLE);
	else if (m.is_promotion()) this->_handle_promotion(m, current_color, from_piece, set);

	this->_handle_castling_rights(m, from_piece, target_piece);

	if (m.is_capture() && target_piece != piece_set_t::null_iterator) this->_delete_captured_piece(target_piece);
	_move_piece(from_square, to_square, from_piece, set);

	this->moves.push_back(m);
	this->history.push(std::move(old_state));
	this->halfmove++;
	// due to moves like en passant where to_piece is not necessarily on the same square as
	// the target square, we cannot rely on the to_piece iterator to be accurate here.
	// We can't use from_piece either, because it also gets zeroed in _move_piece.
	if (m.is_capture() || m.is_promotion() || *to_piece == PieceType::PAWN) this->fifty_move_clock = 0;
	else this->fifty_move_clock++;

	const piece_set_t &current_pieces = this->pieces[current_color];
	const piece_set_t &enemy_pieces   = this->pieces[other_color];

	set.pieces.visible       = bitboard::generate_piece_visibility(current_pieces, current_color, this->bitboards);
	other_set.pieces.visible = bitboard::generate_piece_visibility(enemy_pieces, other_color, this->bitboards);
	set.threats              = bitboard::generate_threat_lines(*this, current_color, this->bitboards);
	other_set.threats        = bitboard::generate_threat_lines(*this, other_color, this->bitboards);
	this->_in_check          = in_check(this);
}

// FIXME: Pawn list grows after undoing an en passant, causing mayhem in the move generator.

void Board::unmake_move()
{
	IrreversableState last_state = this->history.top();
	Move              last_move  = this->moves.back();
	this->history.pop();
	this->moves.pop_back();

	this->rights = last_state.rights;
	this->en_passant_target     = last_state.en_passant_target;
	this->fifty_move_clock      = last_state.fifty_move_clock;
	Piece captured              = last_state.captured_piece;

	this->halfmove--;

	color_t               current_color = this->turn_to_move();
	color_t               other_color   = invert_color(current_color);
	bitboard::single_set &set           = this->bitboards[current_color];
	bitboard::single_set &other_set     = this->bitboards[other_color];

	_move_piece(last_move.get_to(), last_move.get_from(), this->piece_board.at(last_move.get_to()), set);
	// if (!captured.is_none()) this->add_piece(captured);

	piece_set_t::iterator &moved_piece = this->piece_board.at(last_move.get_from());

	if (last_move.is_promotion()) _handle_undo_promotion(last_move, current_color, moved_piece, set);
	if (!captured.is_none())
	{
		this->add_piece(captured);
		get_piece_bitboard(other_set, captured).set(captured.position());
		other_set.pieces.all_pieces.set(captured.position());
	}

	bool is_castle_move = last_move.get_flags() == move_flags::KINGSIDE_CASTLE
	                      || last_move.get_flags() == move_flags::QUEENSIDE_CASTLE;
	if (is_castle_move) _handle_undo_castling(last_move, last_move.get_flags() == move_flags::KINGSIDE_CASTLE);

	const piece_set_t &current_pieces = this->pieces[current_color];
	const piece_set_t &enemy_pieces   = this->pieces[other_color];

	set.pieces.visible       = bitboard::generate_piece_visibility(current_pieces, current_color, this->bitboards);
	other_set.pieces.visible = bitboard::generate_piece_visibility(enemy_pieces, other_color, this->bitboards);
	set.threats              = bitboard::generate_threat_lines(*this, current_color, this->bitboards);
	other_set.threats        = bitboard::generate_threat_lines(*this, other_color, this->bitboards);
	this->_in_check          = in_check(this);
}

#pragma endregion MOVE_PROCESSING

unsigned int square_to_index(const std::string &square)
{
	char file = square.at(0) - 'a';
	char rank = square.at(1) - '1';
	return rank * 8 + file;
}

std::string index_to_square(unsigned int index)
{
	char file = (index % 8) + 'a';
	char rank = (index / 8) + '1';
	return std::to_string(file) + rank;
}

Board Board::simulate_move(Move m) const
{
	Board new_board(*this);
	new_board.make_move(m);
	return new_board;
}