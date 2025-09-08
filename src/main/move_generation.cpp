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

std::vector<bitboard::bitboard>::const_iterator get_pin_line(size_t piece_index, const bitboard::bb_list &pins)
{
	for (auto l = pins.boards.cbegin(); l != pins.boards.cend(); ++l)
		if (l->test(piece_index)) return l;
	return pins.boards.cend();
}

bool is_en_passant_discovered_check(const Board &state, const Piece &pawn, DirectionOffset offset_to_target)
{
	using namespace bitboard;
	Color enemy_color = invert_color(pawn.get_color());

	const piece_boards &enemy_pieces = enemy_color == Color::WHITE ? state.bitboards.white.pieces
	                                                               : state.bitboards.black.pieces;
	const piece_boards &our_pieces   = enemy_color == Color::WHITE ? state.bitboards.black.pieces
	                                                               : state.bitboards.white.pieces;

	::bitboard::bitboard all_pieces = state.bitboards.white.pieces.all_pieces | state.bitboards.black.pieces.all_pieces;

	const int file        = get_file_from_square(pawn.position());
	const int rank        = get_rank_from_square(pawn.position());
	const int rank_offset = 8 * rank;

	const uint8_t king_position = std::countr_zero((rank_1 << rank_offset & our_pieces.kings).to_ullong());
	if (king_position >= 64) return false;

	bool king_on_left = get_file_from_square(king_position) < file;

	::bitboard::bitboard bits_to_check;
	if (king_on_left) bits_to_check = ((rank_1 << (file + 1)) & rank_1) << rank_offset;
	else bits_to_check = (rank_1 >> (8 - file)) << rank_offset;

	uint64_t sliders = ((enemy_pieces.rooks | enemy_pieces.queens) & bits_to_check).to_ullong();
	if (sliders == 0) return false;

	uint8_t enemy_piece_position = king_on_left ? std::countr_zero(sliders) : 63 - std::countl_zero(sliders);
	int     enemy_file           = get_file_from_square(enemy_piece_position);
	size_t  max_steps            = king_on_left ? enemy_file : 7 - enemy_file;
	if (max_steps == 0) return false;

#ifndef NDEBUG
	if (state.pieces[enemy_piece_position] == piece_set_t::null_iterator)
		throw std::runtime_error("En passant check discovered null piece.");
#endif

	all_pieces.reset(pawn.position()).reset(pawn.position() + (int8_t) get_horizontal(offset_to_target));

	threat_line threat = generate_threat_line(*state.pieces[enemy_piece_position],
	                                          all_pieces,
	                                          enemy_pieces.all_pieces & all_pieces,
	                                          king_position,
	                                          king_on_left ? DirectionOffset::LEFT : DirectionOffset::RIGHT,
	                                          max_steps);

	return threat.line.any();
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
	bitboard::bitboard captures;
	bitboard::bitboard enemy_pieces;

	const bitboard::threat_boards *limiters;

	if (pawn == Color::WHITE)
	{
		captures     = WHITE_PAWN_CAPTURES.at(pawn.position());
		enemy_pieces = bb_set.black.pieces.all_pieces;
		limiters     = &bb_set.white.threats;
	}
	else
	{
		captures     = BLACK_PAWN_CAPTURES.at(pawn.position());
		enemy_pieces = bb_set.white.pieces.all_pieces;
		limiters     = &bb_set.black.threats;
	}

	int16_t en_passant_square = state.get_en_passant_target();
	if (en_passant_square != -1 && captures.test(en_passant_square)
	    && !is_en_passant_discovered_check(state, pawn, (DirectionOffset) (en_passant_square - pawn.position())))
		enemy_pieces.set(en_passant_square);

	captures &= enemy_pieces;

	auto line = get_pin_line(pawn.position(), limiters->pins);

	if (line != limiters->pins.boards.cend())
	{
		if (state.is_in_check()) return;
		captures &= *line;
	}
	else if (limiters->checks.combined.to_ullong()) captures &= limiters->checks.combined;

	uint64_t captures_int = captures.to_ullong();

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
			                   move_flags::CAPTURE | (is_en_passant ? move_flags::EN_PASSANT : 0));
		captures.reset(bit_index);
	}
}

void generate_moves_for_pawn(const Board              &state,
                             std::vector<Move>        &moves,
                             const Piece              &pawn,
                             const bitboard::full_set &bb_set)
{
	// This function needs friendly/enemy pieces, and friendly limiters. If performance is that much of an issue,
	// then those can be passed in directly instead of recalculating it here.

	generate_capture_moves_for_pawn(state, moves, pawn, bb_set);
	int from_file = get_file_from_square(pawn.position());

	bitboard::bitboard bb_move;
	bitboard::bitboard bb_double_move;

	bitboard::bitboard             friendly_pieces;
	bitboard::bitboard             enemy_pieces;
	const bitboard::threat_boards *limiters;

	if (pawn == Color::WHITE)
	{
		bb_move         = WHITE_PAWN_MOVES.at(pawn.position())[0];
		bb_double_move  = WHITE_PAWN_MOVES.at(pawn.position())[1];
		friendly_pieces = bb_set.white.pieces.all_pieces;
		enemy_pieces    = bb_set.black.pieces.all_pieces;
		limiters        = &bb_set.white.threats;
	}
	else
	{
		bb_move         = BLACK_PAWN_MOVES.at(pawn.position())[0];
		bb_double_move  = BLACK_PAWN_MOVES.at(pawn.position())[1];
		friendly_pieces = bb_set.black.pieces.all_pieces;
		enemy_pieces    = bb_set.white.pieces.all_pieces;
		limiters        = &bb_set.black.threats;
	}

	bb_move        &= ~(friendly_pieces | enemy_pieces);
	bb_double_move &= ~(friendly_pieces | enemy_pieces);

	if (bb_move.none()) return;

	auto line = get_pin_line(pawn.position(), limiters->pins);
	if (line != limiters->pins.boards.cend())
	{
		if (state.is_in_check()) return;
		bb_move        &= *line;
		bb_double_move &= *line;
	}
	else if (limiters->checks.combined.any())
	{
		bb_move        &= limiters->checks.combined;
		bb_double_move &= limiters->checks.combined;
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
                               const bitboard::threat_boards &limiters)
{
	bitboard::bitboard moves_bb  = KNIGHT_MOVES.at(knight.position());
	moves_bb                    &= ~friendly_pieces;

	auto line = get_pin_line(knight.position(), limiters.pins);
	if (line != limiters.pins.boards.cend())
	{
		if (state.is_in_check()) return;
		moves_bb &= *line;
	}
	else if (limiters.checks.combined.any()) moves_bb &= limiters.checks.combined;

	uint64_t moves_int = moves_bb.to_ullong();

	// clang-format off
	for (uint16_t bit_index = std::countr_zero<uint64_t>(moves_int);
	     bit_index < sizeof(uint64_t) * 8;
	     bit_index     = std::countr_zero<uint64_t>(moves_int))
	// clang-format on
	{
		bool is_capture = state.pieces.at(bit_index) != piece_set_t::null_iterator;
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
                            const bitboard::threat_boards &limiters)
{
	if (max_steps <= 0) return;
	uint16_t to = piece.position();

	auto               line = get_pin_line(piece.position(), limiters.pins);
	bitboard::bitboard allowed_squares(UINT64_MAX);

	if (line != limiters.pins.boards.cend())
	{
		allowed_squares &= *line;
		if (state.is_in_check() && (allowed_squares & limiters.checks.combined).none()) return;
	}
	else if (limiters.checks.combined.any()) allowed_squares &= limiters.checks.combined;

	if (allowed_squares == 0) return;

	for (size_t i = 0; i < max_steps; i++)
	{
		to                                   += (int16_t) direction;
		piece_set_t::const_iterator to_piece  = state.pieces.at(to);
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
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(bishop.position());

	for (size_t i = 4; i < 8; i++)
		generate_moves_on_line(state, moves, bishop, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

void generate_moves_for_rook(const Board                   &state,
                             std::vector<Move>             &moves,
                             const Piece                   &rook,
                             const bitboard::threat_boards &limiters)
{
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(rook.position());

	for (size_t i = 0; i < 4; i++)
		generate_moves_on_line(state, moves, rook, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

void generate_moves_for_queen(const Board                   &state,
                              std::vector<Move>             &moves,
                              const Piece                   &queen,
                              const bitboard::threat_boards &limiters)
{
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(queen.position());

	for (size_t i = 0; i < DIRECTION_OFFSETS.size(); i++)
		generate_moves_on_line(state, moves, queen, DIRECTION_OFFSETS[i], squares_to_edge[i], limiters);
}

#pragma endregion SLIDING_MOVES
#pragma region KING_MOVES

void generate_castling_moves(const Board              &state,
                             const Piece              &king,
                             const bitboard::full_set &bitboards,
                             bool                      kingside,
                             bool                      is_white,
                             std::vector<Move>        &moves)
{
	if (state.is_in_check()) return;
	Board::CastlingRights rights = is_white ? state.get_white_castling_rights() : state.get_black_castling_rights();

	if (kingside && !rights.kingside) return;
	if (!kingside && !rights.queenside) return;

	DirectionOffset kingside_offset  = DirectionOffset::RIGHT;
	DirectionOffset queenside_offset = DirectionOffset::LEFT;

	const bitboard::bitboard &all_pieces       = bitboards.white.pieces.all_pieces | bitboards.black.pieces.all_pieces;
	const bitboard::bitboard &enemy_visibility = is_white ? bitboards.black.pieces.visible
	                                                      : bitboards.white.pieces.visible;

	if (kingside)
	{
		const uint16_t first_square  = king.position() + (int) kingside_offset;
		const uint16_t second_square = first_square + (int) kingside_offset;

#ifndef NDEBUG
		piece_set_t::const_iterator rook = state.pieces.at(second_square + (int) kingside_offset);
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
		piece_set_t::const_iterator rook = state.pieces.at(third_square + (int) queenside_offset);
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
	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(king.position());

	const bitboard::bitboard &enemy_visibility = king.get_color() == Color::WHITE ? bitboards.black.pieces.visible
	                                                                              : bitboards.white.pieces.visible;

	for (size_t i = 0; i < DIRECTION_OFFSETS.size(); i++)
	{
		if (squares_to_edge[i] == 0) continue;
		const uint16_t              to       = king.position() + (int) DIRECTION_OFFSETS[i];
		piece_set_t::const_iterator to_piece = state.pieces.at(to);
		if (to_piece != piece_set_t::null_iterator && to_piece->get_color() == king.get_color()) continue;
		if (enemy_visibility.test(to)) continue;
		moves.emplace_back(king.position(),
		                   to,
		                   to_piece != piece_set_t::null_iterator ? move_flags::CAPTURE : move_flags::QUIET_MOVE);
	}

	generate_castling_moves(state, king, bitboards, true, king.get_color() == Color::WHITE, moves);
	generate_castling_moves(state, king, bitboards, false, king.get_color() == Color::WHITE, moves);
}

#pragma endregion KING_MOVES

std::vector<Move> generate_moves(const Board &state)
{
	std::vector<Move> moves;
	moves.reserve(MAX_MOVES_PER_BOARD);
	const bitboard::full_set &bitboards = state.get_bitboards();

	Color              current_color         = state.turn_to_move();
	const piece_set_t &our_piece_set         = current_color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const piece_set_t &enemy_piece_set       = current_color == Color::WHITE ? state.black_pieces : state.white_pieces;
	const bitboard::single_set &our_bb_set   = current_color == Color::WHITE ? bitboards.white : bitboards.black;
	const bitboard::single_set &enemy_bb_set = current_color == Color::WHITE ? bitboards.black : bitboards.white;

	bool double_check = our_bb_set.threats.checks.boards.size() > 1;

	generate_moves_for_king(state, moves, our_piece_set.kings.front(), bitboards);
	if (double_check) return moves;

	for (auto &queen : our_piece_set.queens)
	{
		assert(queen.get_type() == PieceType::QUEEN && "Piece type mismatch in queen piece set.");
		generate_moves_for_queen(state, moves, queen, our_bb_set.threats);
	}
	for (auto &rook : our_piece_set.rooks)
	{
		assert(rook.get_type() == PieceType::ROOK && "Piece type mismatch in rook piece set.");
		generate_moves_for_rook(state, moves, rook, our_bb_set.threats);
	}
	for (auto &bishop : our_piece_set.bishops)
	{
		assert(bishop.get_type() == PieceType::BISHOP && "Piece type mismatch in bishop piece set.");
		generate_moves_for_bishop(state, moves, bishop, our_bb_set.threats);
	}
	for (auto &knight : our_piece_set.knights)
	{
		assert(knight.get_type() == PieceType::KNIGHT && "Piece type mismatch in knight piece set.");
		generate_moves_for_knight(state, moves, knight, our_bb_set.pieces.all_pieces, our_bb_set.threats);
	}
	for (auto &pawn : our_piece_set.pawns)
	{
		assert(pawn.get_type() == PieceType::PAWN && "Piece type mismatch in pawn piece set.");
		generate_moves_for_pawn(state, moves, pawn, bitboards);
	}

	return moves;
}