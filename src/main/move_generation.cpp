#include "move_generation.hpp"
#include "bitboard.hpp"
#include "board.hpp"
#include "move.hpp"
#include "pieces.hpp"
#include <bit>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

std::vector<bitboard::bitboard>::const_iterator get_pin_line(size_t piece_index, const bitboard::list &pins)
{
	for (auto l = pins.boards.cbegin(); l != pins.boards.cend(); ++l)
		if (l->test(piece_index)) return l;
	return pins.boards.cend();
}

// Generates a line from (start, end)
bitboard::bitboard generate_line(uint8_t start, uint8_t end)
{
	if (start == end) return 0;
	else if (end > start) return (1ULL << end) - (2ULL << start);
	else return (1ULL << start) - (2ULL << end);
}

bool is_en_passant_discovered_check(const Board &state, const Piece &pawn, uint8_t target_position)
{
	using bitboard::piece_boards;
	using bitboard::rank_1;

	const uint8_t       current_color  = (uint8_t) pawn.get_color();
	const uint8_t       other_color    = (uint8_t) invert_color(pawn.get_color());
	const piece_boards &current_pieces = state.bitboards[current_color].pieces;
	const piece_boards &other_pieces   = state.bitboards[other_color].pieces;

	bitboard::bitboard bits_to_check, all_pieces = current_pieces.all_pieces | other_pieces.all_pieces;

	const int file        = get_file_from_square(pawn.position());
	const int rank        = get_rank_from_square(pawn.position());
	const int rank_offset = 8 * rank;

	const uint8_t king_position = std::countr_zero((rank_1 << rank_offset & current_pieces.kings).to_ullong());
	if (king_position >= 64) return false;

	const bool king_on_left = get_file_from_square(king_position) < file;
	if (king_on_left) bits_to_check = ((rank_1 << (file + 1)) & rank_1) << rank_offset;
	else bits_to_check = (rank_1 >> (8 - file)) << rank_offset;

	uint64_t sliders = ((other_pieces.rooks | other_pieces.queens) & bits_to_check).to_ullong();
	if (sliders == 0) return false;

	uint8_t slider_position = king_on_left ? std::countr_zero(sliders) : 63 - std::countl_zero(sliders);

#ifndef NDEBUG
	if (state.piece_board[slider_position] == piece_set_t::null_iterator)
		throw std::runtime_error("En passant check discovered null piece.");
#endif

	all_pieces.reset(pawn.position()) &= ~(bitboard::file_a << get_file_from_square(target_position));
	bitboard::bitboard slider_to_king  = generate_line(king_position, slider_position);
	return (slider_to_king & all_pieces).none();
}

#pragma region PAWN_MOVES

void generate_promotion_moves_for_pawn(std::vector<Move> &moves, uint16_t from, uint16_t to, bool is_capture)
{
	for (uint8_t type = 0; type < (uint8_t) PromotionOptions::MAX_OPTIONS; type++)
		moves.emplace_back(from, to, move_flags::PROMOTION | type | (is_capture ? move_flags::CAPTURE : 0));
}

void generate_capture_moves_for_pawn(const Board              &state,
                                     std::vector<Move>        &moves,
                                     const Piece              &pawn,
                                     const bitboard::full_set &bb_set)
{
	const uint8_t      current_color = (uint8_t) pawn.get_color();
	const uint8_t      other_color   = (uint8_t) invert_color(pawn.get_color());
	bitboard::bitboard captures      = PAWN_CAPTURES[current_color][pawn.position()];
	bitboard::bitboard enemy_pieces  = bb_set[other_color].pieces.all_pieces;

	const bitboard::threat_boards &threats = bb_set[current_color].threats;

	int16_t en_passant_square = state.get_en_passant_target();
	if (en_passant_square != -1 && captures.test(en_passant_square)
	    && !is_en_passant_discovered_check(state, pawn, en_passant_square))
		enemy_pieces.set(en_passant_square);

	captures &= enemy_pieces;

	auto line = get_pin_line(pawn.position(), threats.pins);

	if (line != threats.pins.boards.cend())
	{
		if (state.is_in_check()) return;
		captures &= *line;
	}
	else if (threats.checks.combined.to_ullong()) captures &= threats.checks.combined;

	// clang-format off
	for (uint16_t bit_index = std::countr_zero<uint64_t>(captures.bits);
	     bit_index < sizeof(uint64_t) * 8;
	     bit_index     = std::countr_zero<uint64_t>(captures.bits))
	// clang-format on
	{
		bool is_promotion  = ((bit_index + 8) % 64) < 16;
		bool is_en_passant = bit_index == en_passant_square;
		// For loop already checks the bounds. No need to check here, too.
		if (is_promotion) generate_promotion_moves_for_pawn(moves, pawn.position(), bit_index, true);
		else
			moves.emplace_back(pawn.position(),
			                   bit_index,
			                   (is_en_passant ? move_flags::EN_PASSANT : move_flags::CAPTURE));
		captures.reset(bit_index);
	}
}

void generate_moves_for_pawn(const Board              &state,
                             std::vector<Move>        &moves,
                             const Piece              &pawn,
                             const bitboard::full_set &bb_set)
{
	int from_file = get_file_from_square(pawn.position());
	int from_rank = get_rank_from_square(pawn.position());
	if (from_rank == 0 || from_rank == 7)
	{
#ifndef NDEBUG
		throw std::runtime_error("Invalid pawn rank.");
#else
		return;
#endif
	}
	// This function needs friendly/enemy pieces, and friendly limiters. If performance is that much of an issue,
	// then those can be passed in directly instead of recalculating it here.

	uint8_t pawn_color  = (uint8_t) pawn.get_color();
	uint8_t other_color = (uint8_t) invert_color(pawn.get_color());

	generate_capture_moves_for_pawn(state, moves, pawn, bb_set);

	bitboard::bitboard bb_move{ 0 };
	bitboard::bitboard bb_double_move{ 0 };

	bb_move.set(pawn.position() + (int) PAWN_MOVE_OFFSETS[pawn_color]);
	if (from_rank == PAWN_DOUBLE_MOVE_RANKS[pawn_color])
		bb_double_move.set(pawn.position() + (int) PAWN_MOVE_OFFSETS[pawn_color] * 2);

	bitboard::bitboard             friendly_pieces = bb_set[pawn_color].pieces.all_pieces;
	bitboard::bitboard             enemy_pieces    = bb_set[other_color].pieces.all_pieces;
	const bitboard::threat_boards &threat_boards   = bb_set[pawn_color].threats;

	bb_move        &= ~(friendly_pieces | enemy_pieces);
	bb_double_move &= ~(friendly_pieces | enemy_pieces);

	if (bb_move.none()) return;

	auto line = get_pin_line(pawn.position(), threat_boards.pins);
	if (line != threat_boards.pins.boards.cend())
	{
		if (state.is_in_check()) return;
		bb_move        &= *line;
		bb_double_move &= *line;
	}
	else if (threat_boards.checks.combined.any())
	{
		bb_move        &= threat_boards.checks.combined;
		bb_double_move &= threat_boards.checks.combined;
	}

	uint16_t square_index = std::countr_zero(bb_move.to_ullong());
	bool     is_promotion = ((square_index + 8) % 64) < 16;

	if (square_index != 64)
	{
		if (is_promotion) generate_promotion_moves_for_pawn(moves, pawn.position(), square_index, false);
		else moves.emplace_back(pawn.position(), square_index);
	}

	if (bb_double_move == 0) return;

	square_index = std::countr_zero(bb_double_move.to_ullong());
	moves.emplace_back(pawn.position(), square_index, move_flags::DOUBLE_PAWN_PUSH);
}

#pragma endregion PAWN_MOVES
#pragma region KNIGHT_MOVES

void generate_moves_for_knight(const Board                   &state,
                               std::vector<Move>             &moves,
                               const Piece                   &knight,
                               const bitboard::bitboard      &friendly_pieces,
                               const bitboard::threat_boards &threats)
{
	bitboard::bitboard moves_bb = KNIGHT_MOVES[knight.position()] & ~friendly_pieces;

	auto line = get_pin_line(knight.position(), threats.pins);
	if (line != threats.pins.boards.cend())
	{
		if (state.is_in_check()) return;
		moves_bb &= *line;
	}
	else if (threats.checks.combined.any()) moves_bb &= threats.checks.combined;

	uint64_t moves_int = moves_bb.to_ullong();

	// clang-format off
	for (uint16_t bit_index = std::countr_zero<uint64_t>(moves_int);
	     bit_index < sizeof(uint64_t) * 8;
	     bit_index     = std::countr_zero<uint64_t>(moves_int))
	// clang-format on
	{
		bool is_capture = state.piece_board.at(bit_index) != piece_set_t::null_iterator;
		moves.emplace_back(knight.position(), bit_index, is_capture ? move_flags::CAPTURE : move_flags::QUIET_MOVE);
		moves_int ^= 1ULL << bit_index;
	}
}

#pragma endregion KNIGHT_MOVES
#pragma region SLIDING_MOVES

void generate_moves_on_line(const Board                   &state,
                            std::vector<Move>             &moves,
                            const Piece                   &piece,
                            DirectionOffset                direction,
                            size_t                         max_steps,
                            const bitboard::threat_boards &threats)
{
	if (max_steps <= 0) return;
	uint16_t to = piece.position();

	auto               line = get_pin_line(piece.position(), threats.pins);
	bitboard::bitboard allowed_squares(UINT64_MAX);

	if (line != threats.pins.boards.cend())
	{
		allowed_squares &= *line;
		if (state.is_in_check() && (allowed_squares & threats.checks.combined).none()) return;
	}
	else if (threats.checks.combined.any()) allowed_squares &= threats.checks.combined;

	if (allowed_squares == 0) return;

	for (size_t i = 0; i < max_steps; i++)
	{
		to                                   += (int16_t) direction;
		piece_set_t::const_iterator to_piece  = state.piece_board[to];
		if (to_piece != piece_set_t::null_iterator)
		{
			if (to_piece->get_color() == piece.get_color()) break;
			if (!allowed_squares.test(to)) break;
			moves.emplace_back(piece.position(), to, move_flags::CAPTURE);
			break;
		}
		if (!allowed_squares.test(to)) continue;
		moves.emplace_back(piece.position(), to);
	}
}

void generate_moves_for_bishop(const Board                   &state,
                               std::vector<Move>             &moves,
                               const Piece                   &bishop,
                               const bitboard::threat_boards &limiters)
{
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE[bishop.position()];

	for (size_t i = 4; i < 8; i++)
		generate_moves_on_line(state, moves, bishop, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

void generate_moves_for_rook(const Board                   &state,
                             std::vector<Move>             &moves,
                             const Piece                   &rook,
                             const bitboard::threat_boards &limiters)
{
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE[rook.position()];

	for (size_t i = 0; i < 4; i++)
		generate_moves_on_line(state, moves, rook, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

void generate_moves_for_queen(const Board                   &state,
                              std::vector<Move>             &moves,
                              const Piece                   &queen,
                              const bitboard::threat_boards &limiters)
{
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE[queen.position()];

	for (size_t i = 0; i < DIRECTION_OFFSETS.size(); i++)
		generate_moves_on_line(state, moves, queen, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

#pragma endregion SLIDING_MOVES
#pragma region KING_MOVES

void generate_castling_moves(const Board              &state,
                             const Piece              &king,
                             const bitboard::full_set &bitboards,
                             bool                      kingside,
                             std::vector<Move>        &moves)
{
	if (state.is_in_check()) return;
	Board::CastlingRights rights = state.get_castling_rights(king.get_color());

	if (kingside && !rights.kingside) return;
	if (!kingside && !rights.queenside) return;

	DirectionOffset kingside_offset  = DirectionOffset::RIGHT;
	DirectionOffset queenside_offset = DirectionOffset::LEFT;

	const bitboard::bitboard &all_pieces       = bitboards[0].pieces.all_pieces | bitboards[1].pieces.all_pieces;
	const bitboard::bitboard &enemy_visibility = bitboards[invert_color(king.get_color())].pieces.visible;

	if (kingside)
	{
		const uint16_t first_square  = king.position() + (int) kingside_offset;
		const uint16_t second_square = first_square + (int) kingside_offset;

#ifndef NDEBUG
		piece_set_t::const_iterator rook = state.piece_board.at(second_square + (int) kingside_offset);
		assert(rook != piece_set_t::null_iterator && *rook == PieceType::ROOK
		       && "Move generation: tried king castling without a rook");
#endif

		bitboard::bitboard mask = bitboard::bitboard().set(first_square).set(second_square);
		// uint64_t mask = 1ULL << first_square | 1ULL << second_square;
		if (((enemy_visibility | all_pieces) & mask).any()) return;
		moves.emplace_back(king.position(), second_square, move_flags::KINGSIDE_CASTLE);
	}
	else
	{
		const uint16_t first_square  = king.position() + (int) queenside_offset;
		const uint16_t second_square = first_square + (int) queenside_offset;
		const uint16_t third_square  = second_square + (int) queenside_offset;

#ifndef NDEBUG
		piece_set_t::const_iterator rook = state.piece_board.at(third_square + (int) queenside_offset);
		assert(rook != piece_set_t::null_iterator && *rook == PieceType::ROOK
		       && "Move generation: tried queen castling without a rook");
#endif

		bitboard::bitboard mask       = bitboard::bitboard().set(first_square).set(second_square);
		// uint64_t mask       = 1ULL << first_square | 1ULL << second_square;
		bitboard::bitboard clear_mask = bitboard::bitboard(mask).set(third_square);
		// uint64_t clear_mask = mask | 1ULL << third_square;
		if ((enemy_visibility & mask).any()) return;
		if ((all_pieces & clear_mask).any()) return;
		moves.emplace_back(king.position(), second_square, move_flags::QUEENSIDE_CASTLE);
	}
}

void generate_moves_for_king(const Board              &state,
                             std::vector<Move>        &moves,
                             const Piece              &king,
                             const bitboard::full_set &bitboards)
{
	uint8_t our_color_int   = (uint8_t) king.get_color();
	uint8_t other_color_int = (uint8_t) invert_color(king.get_color());

	const std::array<size_t, 8> &squares_to_edge  = NUM_SQUARES_TO_EDGE.at(king.position());
	const bitboard::bitboard    &our_pieces       = bitboards[our_color_int].pieces.all_pieces;
	const bitboard::bitboard    &enemy_pieces     = bitboards[other_color_int].pieces.all_pieces;
	const bitboard::bitboard    &enemy_visibility = bitboards[other_color_int].pieces.visible;

	for (size_t i = 0; i < DIRECTION_OFFSETS.size(); i++)
	{
		if (squares_to_edge[i] == 0) continue;
		const uint8_t to = king.position() + (int) DIRECTION_OFFSETS[i];
		if (our_pieces.test(to) || enemy_visibility.test(to)) continue;
		moves.emplace_back(king.position(), to, enemy_pieces.test(to) ? move_flags::CAPTURE : move_flags::QUIET_MOVE);
	}

	generate_castling_moves(state, king, bitboards, true, moves);
	generate_castling_moves(state, king, bitboards, false, moves);
}

#pragma endregion KING_MOVES

std::vector<Move> generate_moves(const Board &state)
{
	std::vector<Move> moves;
	moves.reserve(MAX_MOVES_PER_BOARD);
	const bitboard::full_set &bitboards = state.get_bitboards();

	const uint8_t               current_color  = (uint8_t) state.turn_to_move();
	const uint8_t               other_color    = (uint8_t) invert_color(state.turn_to_move());
	const piece_set_t          &current_pieces = state.pieces[current_color];
	const piece_set_t          &other_pieces   = state.pieces[other_color];
	const bitboard::single_set &current_boards = bitboards[current_color];
	const bitboard::single_set &other_boards   = bitboards[other_color];

	bool double_check = current_boards.threats.checks.boards.size() > 1;

	generate_moves_for_king(state, moves, current_pieces.kings.front(), bitboards);
	if (double_check) return moves;

	for (auto &queen : current_pieces.queens)
	{
		assert(queen.get_type() == PieceType::QUEEN && "Piece type mismatch in queen piece set.");
		generate_moves_for_queen(state, moves, queen, current_boards.threats);
	}
	for (auto &rook : current_pieces.rooks)
	{
		assert(rook.get_type() == PieceType::ROOK && "Piece type mismatch in rook piece set.");
		generate_moves_for_rook(state, moves, rook, current_boards.threats);
	}
	for (auto &bishop : current_pieces.bishops)
	{
		assert(bishop.get_type() == PieceType::BISHOP && "Piece type mismatch in bishop piece set.");
		generate_moves_for_bishop(state, moves, bishop, current_boards.threats);
	}
	for (auto &knight : current_pieces.knights)
	{
		assert(knight.get_type() == PieceType::KNIGHT && "Piece type mismatch in knight piece set.");
		generate_moves_for_knight(state, moves, knight, current_boards.pieces.all_pieces, current_boards.threats);
	}
	for (auto &pawn : current_pieces.pawns)
	{
		assert(pawn.get_type() == PieceType::PAWN && "Piece type mismatch in pawn piece set.");
		generate_moves_for_pawn(state, moves, pawn, bitboards);
	}

	return moves;
}