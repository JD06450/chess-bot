#include "bitboard.hpp"

#include "board.hpp"
#include "move.hpp"
#include "move_generation.hpp"
#include "pieces.hpp"
#include <cassert>
#include <cstddef>
#include <sstream>
#include <stdexcept>

namespace bitboard
{

#pragma region Piece_Boards

bitboard generate_piece_board(const piece_set_t::PieceList &list)
{
	bitboard board;
	for (auto &piece : list) board.set(piece.position());
	return board;
}

// bitboard generate_king_board(const Board &state, const piece_set_t &set)
// {
// 	bitboard    board;
// 	const Piece &king = set.kings.front();
// 	board.set(king.position());
// 	return board;
// }

piece_boards generate_piece_boards(const Board &state, color_t color)
{
	const piece_set_t &set = state.pieces[(size_t) color];

	piece_boards pieces = { generate_piece_board(set.pawns),   generate_piece_board(set.knights),
		                    generate_piece_board(set.bishops), generate_piece_board(set.rooks),
		                    generate_piece_board(set.queens),  generate_piece_board(set.kings) };
	pieces.visible = generate_piece_visibility(set, color, state.get_bitboards());
	pieces.calculate_combined();
	return pieces;
}

#pragma endregion Piece_Boards

#pragma region Piece_Visibility

bitboard generate_pawn_visibility(const Piece &pawn) { return PAWN_CAPTURES[pawn.get_color()][pawn.position()]; }

bitboard generate_knight_visibility(const Piece &knight) { return KNIGHT_MOVES[knight.position()]; }

bitboard generate_visibility_on_line(const Piece    &piece,
                                     DirectionOffset direction,
                                     size_t          max_steps,
                                     bitboard        break_board)
{
	bitboard moves(0);
	if (max_steps == 0) return moves;
	// const Piece enemy_king = Piece(PieceType::KING, oc);
	int8_t to = piece.position();

	for (size_t i = 0; i < max_steps; i++)
	{
		to += (int8_t) direction;
		moves.set(to);
		// piece_set_t::const_iterator to_piece = state.pieces[to];
		// piece_set_t::const_iterator to_piece = state.pieces.at(to);
		// if (to_piece != piece_set_t::null_iterator && !to_piece->is_same(enemy_king)) break;
		if (break_board.test(to)) break;
	}

	return moves;
}

bitboard generate_bishop_visibility(const Piece &bishop, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE[bishop.position()];

	for (size_t i = 4; i < 8; i++)
		moves |= generate_visibility_on_line(bishop, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_rook_visibility(const Piece &rook, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(rook.position());

	for (size_t i = 0; i < 4; i++)
		moves |= generate_visibility_on_line(rook, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_queen_visibility(const Piece &queen, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(queen.position());

	for (size_t i = 0; i < 8; i++)
		moves |= generate_visibility_on_line(queen, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_king_visibility(const Piece &king)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(king.position());

	for (size_t i = 0; i < DIRECTION_OFFSETS.size(); i++)
	{
		if (squares_to_edge[i] == 0) continue;
		moves.set(king.position() + (int) DIRECTION_OFFSETS[i]);
	}

	return moves;
}

bitboard generate_piece_visibility(const piece_set_t &piece_set, color_t color, const full_set &old_boards)
{
	const piece_boards &our_bb_set   = old_boards[color].pieces;
	const piece_boards &enemy_bb_set = old_boards[invert_color(color)].pieces;

	bitboard break_board = (our_bb_set.all_pieces | enemy_bb_set.all_pieces) & ~enemy_bb_set.kings;
	bitboard visibility  = generate_king_visibility(piece_set.kings.front());

	for (auto &queen : piece_set.queens)
	{
		assert(queen.get_type() == PieceType::QUEEN && "Piece type mismatch in queen piece set.");
		visibility |= generate_queen_visibility(queen, break_board);
	}
	for (auto &rook : piece_set.rooks)
	{
		assert(rook.get_type() == PieceType::ROOK && "Piece type mismatch in rook piece set.");
		visibility |= generate_rook_visibility(rook, break_board);
	}
	for (auto &bishop : piece_set.bishops)
	{
		assert(bishop.get_type() == PieceType::BISHOP && "Piece type mismatch in bishop piece set.");
		visibility |= generate_bishop_visibility(bishop, break_board);
	}
	for (auto &knight : piece_set.knights)
	{
		assert(knight.get_type() == PieceType::KNIGHT && "Piece type mismatch in knight piece set.");
		visibility |= generate_knight_visibility(knight);
	}
	for (auto &pawn : piece_set.pawns)
	{
		assert(pawn.get_type() == PieceType::PAWN && "Piece type mismatch in pawn piece set.");
		visibility |= generate_pawn_visibility(pawn);
	}

	return visibility;
}

#pragma endregion Piece_Visibility

#pragma region Checks

void generate_checks_for_pawn(const Piece &pawn, const Piece &enemy_king, threat_boards &threats)
{
	bitboard captures = PAWN_CAPTURES[pawn.get_color()][pawn.position()];

	if (captures.test(enemy_king.position())) threats.checks.boards.push_back(bitboard().set(pawn.position()));
}

void generate_checks_for_knight(const Piece &knight, const Piece &enemy_king, threat_boards &threats)
{
	if (KNIGHT_MOVES[knight.position()].test(enemy_king.position()))
		threats.checks.boards.push_back(bitboard().set(knight.position()));
}

// Returns `true` if the line is a check, and `false` when it's a pin.
threat_line generate_threat_line(const Piece    &piece,
                                 bitboard        all_pieces,
                                 bitboard        our_pieces,
                                 uint8_t         enemy_king_pos,
                                 DirectionOffset direction,
                                 size_t          max_steps)
{
#ifndef NDEBUG
	if (max_steps == 0) throw std::invalid_argument("Max steps must be greater than zero.");
#endif
	threat_line new_line{};
	int         to = piece.position();
	new_line.line.set(to);
	new_line.is_check = true;

	// Using magic bitboards would reduce the number of branches here
	// but I don't feel like implementing them right now.
	for (size_t i = 0; i < max_steps; i++)
	{
		to += (int) direction;
#ifndef NDEBUG
		if (to >= 64) throw std::runtime_error("Generating check line outside of the board.");
#endif

		if (enemy_king_pos == to) return new_line;
		if (our_pieces.test(to)) break;

		if (all_pieces.test(to))
		{
			if (!new_line.is_check) break;
			new_line.is_check = false;
		}
		new_line.line.set(to);
	}

	return threat_line{ 0, false };
}

void generate_threats_for_bishop(const Piece   &bishop,
                                 bitboard       all_pieces,
                                 bitboard       our_pieces,
                                 uint8_t        enemy_king_pos,
                                 threat_boards &threats)
{
	const std::array<size_t, 8> squares_to_edge = NUM_SQUARES_TO_EDGE.at(bishop.position());

	for (size_t i = 4; i < DIRECTION_OFFSETS.size(); i++)
	{
		if (squares_to_edge[i] == 0) continue;
		threat_line line = generate_threat_line(bishop,
		                                        all_pieces,
		                                        our_pieces,
		                                        enemy_king_pos,
		                                        DIRECTION_OFFSETS[i],
		                                        squares_to_edge[i]);

		if (line.line.none()) continue;
		if (line.is_check) threats.checks.boards.push_back(line.line);
		else threats.pins.boards.push_back(line.line);
	}
}

void generate_threats_for_rook(const Piece   &rook,
                               bitboard       all_pieces,
                               bitboard       our_pieces,
                               uint8_t        enemy_king_pos,
                               threat_boards &threats)
{
	const std::array<size_t, 8> squares_to_edge = NUM_SQUARES_TO_EDGE.at(rook.position());

	for (size_t i = 0; i < 4; i++)
	{
		if (squares_to_edge[i] == 0) continue;
		threat_line threat = generate_threat_line(rook,
		                                          all_pieces,
		                                          our_pieces,
		                                          enemy_king_pos,
		                                          DIRECTION_OFFSETS[i],
		                                          squares_to_edge[i]);

		if (threat.line.none()) continue;
		if (threat.is_check) threats.checks.boards.push_back(threat.line);
		else threats.pins.boards.push_back(threat.line);
	}
}

threat_boards generate_threat_lines(const Board &state, color_t color, const full_set &old_boards)
{
	threat_boards      threats{};
	const piece_set_t &our_piece_set   = state.pieces[invert_color(color)];
	const piece_set_t &enemy_piece_set = state.pieces[color];
	bitboard           our_bitboards   = old_boards[invert_color(color)].pieces.all_pieces;
	// bitboard           enemy_bb_set    = color == COLOR_WHITE ? old_boards.white.pieces.all_pieces
	//                                                            : old_boards.black.pieces.all_pieces;

	bitboard all_pieces     = old_boards[WHITE].pieces.all_pieces | old_boards[BLACK].pieces.all_pieces;
	uint8_t  enemy_king_pos = enemy_piece_set.kings.front().position();

	for (auto &queen : our_piece_set.queens)
	{
		generate_threats_for_rook(queen, all_pieces, our_bitboards, enemy_king_pos, threats);
		generate_threats_for_bishop(queen, all_pieces, our_bitboards, enemy_king_pos, threats);
	}
	for (auto &rook : our_piece_set.rooks)
		generate_threats_for_rook(rook, all_pieces, our_bitboards, enemy_king_pos, threats);
	for (auto &bishop : our_piece_set.bishops)
		generate_threats_for_bishop(bishop, all_pieces, our_bitboards, enemy_king_pos, threats);
	for (auto &knight : our_piece_set.knights)
		generate_checks_for_knight(knight, enemy_piece_set.kings.front(), threats);
	for (auto &pawn : our_piece_set.pawns) generate_checks_for_pawn(pawn, enemy_piece_set.kings.front(), threats);

	threats.checks.calculate_combined();
	threats.pins.calculate_combined();

	return threats;
}

#pragma endregion Checks

#pragma region Sets

single_set generate_single_set(const Board &state, color_t color)
{
	return {
		generate_piece_boards(state, color),
		generate_threat_lines(state, color, state.get_bitboards())
	};
}

full_set generate_full_set(const Board &state)
{
	full_set new_set{};
	new_set[WHITE].pieces = generate_piece_boards(state, WHITE);
	new_set[BLACK].pieces = generate_piece_boards(state, BLACK);

	new_set[WHITE].pieces.visible = generate_piece_visibility(state.pieces[WHITE], WHITE, new_set);
	new_set[BLACK].pieces.visible = generate_piece_visibility(state.pieces[BLACK], BLACK, new_set);

	new_set[WHITE].threats = generate_threat_lines(state, WHITE, new_set);
	new_set[BLACK].threats = generate_threat_lines(state, BLACK, new_set);

	return new_set;
}

#pragma endregion Sets

std::string to_string(bitboard bb)
{
	std::ostringstream stream;

	for (int rank = 7; rank >= 0; rank--)
	{
		for (int file = 0; file < 8; file++) { stream << (bb.test(rank * 8 + file) ? '#' : '.') << ' '; }
		stream << '\n';
	}
	return stream.str();
}

}