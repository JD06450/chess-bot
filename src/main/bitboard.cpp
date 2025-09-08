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

bitboard generate_pawn_board(const Board &state, Color color)
{
	bitboard           board;
	const piece_set_t &set   = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const auto        &pawns = set.pawns;
	for (auto &pawn : pawns) board.set(pawn.position());
	return board;
}

bitboard generate_knight_board(const Board &state, Color color)
{
	bitboard           board;
	const piece_set_t &set     = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const auto        &knights = set.knights;
	for (auto &knight : knights) board.set(knight.position());
	return board;
}

bitboard generate_bishop_board(const Board &state, Color color)
{
	bitboard           board;
	const piece_set_t &set     = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const auto        &bishops = set.bishops;
	for (auto &bishop : bishops) board.set(bishop.position());
	return board;
}

bitboard generate_rook_board(const Board &state, Color color)
{
	bitboard           board;
	const piece_set_t &set   = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const auto        &rooks = set.rooks;
	for (auto &rook : rooks) board.set(rook.position());
	return board;
}

bitboard generate_queen_board(const Board &state, Color color)
{
	bitboard           board;
	const piece_set_t &set    = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	const auto        &queens = set.queens;
	for (auto &queen : queens) board.set(queen.position());
	return board;
}

bitboard generate_king_board(const Board &state, Color color)
{
	bitboard    board;
	const auto &king = color == Color::WHITE ? state.white_pieces.kings.front() : state.black_pieces.kings.front();
	board.set(king.position());
	return board;
}

piece_boards generate_piece_board(const Board &state, Color color)
{
	piece_boards pieces = { generate_pawn_board(state, color),   generate_knight_board(state, color),
		                    generate_bishop_board(state, color), generate_rook_board(state, color),
		                    generate_queen_board(state, color),  generate_king_board(state, color) };
	pieces.calculate_combined();
	return pieces;
}

#pragma endregion Piece_Boards

#pragma region Piece_Visibility

bitboard generate_pawn_visibility(const Board &state, const Piece &pawn)
{
	return pawn == Color::WHITE ? WHITE_PAWN_CAPTURES[pawn.position()] : BLACK_PAWN_CAPTURES[pawn.position()];
}

bitboard generate_knight_visibility(const Piece &knight) { return KNIGHT_MOVES[knight.position()]; }

bitboard generate_visibility_on_line(const Board    &state,
                                     const Piece    &piece,
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

bitboard generate_bishop_visibility(const Board &state, const Piece &bishop, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE[bishop.position()];

	for (size_t i = 4; i < 8; i++)
		moves |= generate_visibility_on_line(state, bishop, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_rook_visibility(const Board &state, const Piece &rook, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(rook.position());

	for (size_t i = 0; i < 4; i++)
		moves |= generate_visibility_on_line(state, rook, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_queen_visibility(const Board &state, const Piece &queen, bitboard break_board)
{
	bitboard moves(0);

	const std::array<size_t, 8> &squares_to_edge = NUM_SQUARES_TO_EDGE.at(queen.position());

	for (size_t i = 0; i < 8; i++)
		moves |= generate_visibility_on_line(state, queen, DIRECTION_OFFSETS[i], squares_to_edge[i], break_board);

	return moves;
}

bitboard generate_king_visibility(const Board &state, const Piece &king)
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

bitboard generate_piece_visibility(const Board &state, Color color, const full_set &old_boards)
{
	const piece_set_t &piece_set = color == Color::WHITE ? state.white_pieces : state.black_pieces;

	const piece_boards &set       = color == Color::WHITE ? old_boards.white.pieces : old_boards.black.pieces;
	const piece_boards &other_set = color == Color::WHITE ? old_boards.black.pieces : old_boards.white.pieces;

	bitboard break_board = (set.all_pieces | other_set.all_pieces) & ~other_set.kings;

	bitboard visibility = generate_king_visibility(state, piece_set.kings.front());
	size_t   i;

	for (auto &queen : piece_set.queens)
	{
		assert(queen.get_type() == PieceType::QUEEN && "Piece type mismatch in queen piece set.");
		visibility |= generate_queen_visibility(state, queen, break_board);
	}
	for (auto &rook : piece_set.rooks)
	{
		assert(rook.get_type() == PieceType::ROOK && "Piece type mismatch in rook piece set.");
		visibility |= generate_rook_visibility(state, rook, break_board);
	}
	for (auto &bishop : piece_set.bishops)
	{
		assert(bishop.get_type() == PieceType::BISHOP && "Piece type mismatch in bishop piece set.");
		visibility |= generate_bishop_visibility(state, bishop, break_board);
	}
	for (auto &knight : piece_set.knights)
	{
		assert(knight.get_type() == PieceType::KNIGHT && "Piece type mismatch in knight piece set.");
		visibility |= generate_knight_visibility(knight);
	}
	for (auto &pawn : piece_set.pawns)
	{
		assert(pawn.get_type() == PieceType::PAWN && "Piece type mismatch in pawn piece set.");
		visibility |= generate_pawn_visibility(state, pawn);
	}

	return visibility;
}

#pragma endregion Piece_Visibility

#pragma region Checks

void generate_checks_for_pawn(const Piece &pawn, const Piece &enemy_king, threat_boards &limiters)
{
	// int from_file = get_file_from_square(pawn.position());

	// DirectionOffset left_offset  = pawn.get_color() == Color::WHITE ? DirectionOffset::UP_LEFT
	//                                                                 : DirectionOffset::DOWN_LEFT;
	// DirectionOffset right_offset = pawn.get_color() == Color::WHITE ? DirectionOffset::UP_RIGHT
	//                                                                 : DirectionOffset::DOWN_RIGHT;

	// int left_to  = pawn.position() + (int) left_offset;
	// int right_to = pawn.position() + (int) right_offset;

	bitboard captures = pawn.get_color() == Color::WHITE ? WHITE_PAWN_CAPTURES[pawn.position()]
	                                                     : BLACK_PAWN_CAPTURES[pawn.position()];
	// captures &= 1ULL << enemy_king.position();

	// bool left_valid  = left_to == enemy_king.position() && from_file != 0;
	// bool right_valid = right_to == enemy_king.position() && from_file != 7;
	// if (!left_valid && !right_valid) return;

	if (captures.test(enemy_king.position())) limiters.checks.boards.push_back(bitboard(1ULL << pawn.position()));
}

void generate_checks_for_knight(const Piece &knight, const Piece &enemy_king, threat_boards &limiters)
{
	bitboard moves = KNIGHT_MOVES[knight.position()];

	if (moves.test(enemy_king.position())) limiters.checks.boards.push_back(bitboard(1ULL << knight.position()));
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

void generate_threats_for_bishop(const Board   &state,
                                 const Piece   &bishop,
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

void generate_threats_for_rook(const Board   &state,
                               const Piece   &rook,
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

threat_boards generate_threat_lines(const Board &state, Color color, const full_set &old_boards)
{
	threat_boards      threats{};
	const piece_set_t &our_piece_set   = color == Color::WHITE ? state.black_pieces : state.white_pieces;
	const piece_set_t &enemy_piece_set = color == Color::WHITE ? state.white_pieces : state.black_pieces;
	bitboard           our_bb_set      = color == Color::WHITE ? old_boards.black.pieces.all_pieces
	                                                           : old_boards.white.pieces.all_pieces;
	bitboard           enemy_bb_set    = color == Color::WHITE ? old_boards.white.pieces.all_pieces
	                                                           : old_boards.black.pieces.all_pieces;

	bitboard all_pieces     = our_bb_set | enemy_bb_set;
	// bitboard enemy_king = enemy_bb_set.kings;
	uint8_t  enemy_king_pos = enemy_piece_set.kings.front().position();

	for (auto &queen : our_piece_set.queens)
	{
		generate_threats_for_rook(state, queen, all_pieces, our_bb_set, enemy_king_pos, threats);
		generate_threats_for_bishop(state, queen, all_pieces, our_bb_set, enemy_king_pos, threats);
	}
	for (auto &rook : our_piece_set.rooks)
		generate_threats_for_rook(state, rook, all_pieces, our_bb_set, enemy_king_pos, threats);
	for (auto &bishop : our_piece_set.bishops)
		generate_threats_for_bishop(state, bishop, all_pieces, our_bb_set, enemy_king_pos, threats);
	for (auto &knight : our_piece_set.knights)
		generate_checks_for_knight(knight, enemy_piece_set.kings.front(), threats);
	for (auto &pawn : our_piece_set.pawns) generate_checks_for_pawn(pawn, enemy_piece_set.kings.front(), threats);

	threats.checks.calculate_combined();
	threats.pins.calculate_combined();

	return threats;
}

#pragma endregion Checks

#pragma region Sets

single_set generate_bitboard_set(const Board &state, Color color)
{
	single_set set = {
		{
         generate_pawn_board(state, color),
         generate_knight_board(state, color),
         generate_bishop_board(state, color),
         generate_rook_board(state, color),
         generate_queen_board(state, color),
         generate_king_board(state, color),
         bitboard(),
         generate_piece_visibility(state, color, state.get_bitboards()),
		 },
		generate_threat_lines(state, color, state.get_bitboards())
	};

	set.pieces.calculate_combined();
	return set;
}

full_set generate_bitboard_full_set(const Board &state)
{
	full_set new_set{};
	new_set.white.pieces = generate_piece_board(state, Color::WHITE);
	new_set.black.pieces = generate_piece_board(state, Color::BLACK);

	new_set.white.pieces.visible = generate_piece_visibility(state, Color::WHITE, new_set);
	new_set.black.pieces.visible = generate_piece_visibility(state, Color::BLACK, new_set);

	new_set.white.threats = generate_threat_lines(state, Color::WHITE, new_set);
	new_set.black.threats = generate_threat_lines(state, Color::BLACK, new_set);

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