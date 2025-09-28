#include "hc_evaluation.hpp"

#include "bitboard.hpp"
#include "board.hpp"
#include "pieces.hpp"


namespace phase_values
{
using evaluation::eval_t;

constexpr eval_t KNIGHT = 1;
constexpr eval_t BISHOP = 1;
constexpr eval_t ROOK   = 2;
constexpr eval_t QUEEN  = 4;

constexpr eval_t MAX_PIECES  = 2 * QUEEN + 4 * ROOK + 4 * BISHOP + 4 * KNIGHT;
constexpr eval_t HALF_PIECES = MAX_PIECES / 2;
constexpr eval_t MAX_PAWNS   = 16;
constexpr eval_t HALF_PAWNS  = MAX_PAWNS / 2;
}

namespace evaluation::hce
{

eval_t two_phase_lerp(const Board &state, eval_t mg, eval_t eg)
{
	const bitboard::piece_boards &white_bits = state.bitboards[WHITE].pieces;
	const bitboard::piece_boards &black_bits = state.bitboards[BLACK].pieces;

	eval_t phase  = phase_values::MAX_PIECES;
	phase        -= (white_bits.knights | black_bits.knights).count();
	phase        -= (white_bits.bishops | black_bits.bishops).count();
	phase        -= (white_bits.rooks | black_bits.rooks).count();
	phase        -= (white_bits.queens | black_bits.queens).count();
	phase         = (phase * 256) / phase_values::MAX_PIECES;

	return (mg * (phase_values::MAX_PIECES - phase) + eg * phase) / 256;
}

eval_t four_phase_lerp(const Board &state, eval_t p1, eval_t p2, eval_t p3, eval_t p4)
{
	const bitboard::piece_boards &white_bits = state.bitboards[WHITE].pieces;
	const bitboard::piece_boards &black_bits = state.bitboards[BLACK].pieces;

	eval_t piece_phase  = phase_values::MAX_PIECES;
	eval_t pawn_phase   = phase_values::MAX_PAWNS;
	piece_phase        -= (white_bits.knights | black_bits.knights).count();
	piece_phase        -= (white_bits.bishops | black_bits.bishops).count();
	piece_phase        -= (white_bits.rooks | black_bits.rooks).count();
	piece_phase        -= (white_bits.queens | black_bits.queens).count();
	piece_phase         = (piece_phase * 256) / phase_values::MAX_PIECES;
	pawn_phase         -= (white_bits.pawns | black_bits.pawns).count();
	pawn_phase          = (pawn_phase * 256) / phase_values::MAX_PAWNS;

	eval_t final_value  = 0;
	final_value        += (p1 * (512 - piece_phase - pawn_phase));
	final_value        += (p2 * (256 - piece_phase + pawn_phase));
	final_value        += (p3 * (256 + piece_phase - pawn_phase));
	final_value        += (p4 * (0 + piece_phase + pawn_phase));
	return final_value;
}

eval_t evaluate(const Board &state)
{
	eval_t white_piece_eval = 0, black_piece_eval = 0;
	// Start off with simple piece evaluation
	white_piece_eval += state.pieces[WHITE].queens.size() * piece_values::QUEEN_MID;
	white_piece_eval += state.pieces[WHITE].rooks.size() * piece_values::ROOK_MID;
	white_piece_eval += state.pieces[WHITE].bishops.size() * piece_values::BISHOP_MID;
	white_piece_eval += state.pieces[WHITE].knights.size() * piece_values::KNIGHT_MID;
	white_piece_eval += state.pieces[WHITE].pawns.size() * piece_values::PAWN_MID;

	black_piece_eval += state.pieces[BLACK].queens.size() * piece_values::QUEEN_MID;
	black_piece_eval += state.pieces[BLACK].rooks.size() * piece_values::ROOK_MID;
	black_piece_eval += state.pieces[BLACK].bishops.size() * piece_values::BISHOP_MID;
	black_piece_eval += state.pieces[BLACK].knights.size() * piece_values::KNIGHT_MID;
	black_piece_eval += state.pieces[BLACK].pawns.size() * piece_values::PAWN_MID;

	eval_t total_piece_eval = white_piece_eval - black_piece_eval;

	if (state.turn_to_move() == BLACK) total_piece_eval *= -1;
	return total_piece_eval;
}

}