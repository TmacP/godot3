/**************************************************************************/
/*  puzzle_solver.h                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef PUZZLE_SOLVER_H
#define PUZZLE_SOLVER_H

#include "core/reference.h"
#include "core/array.h"
#include "core/dictionary.h"
#include "core/vector.h"
#include "core/hash_map.h"
#include "core/ustring.h"

#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

class PuzzleSolver : public Reference {
	GDCLASS(PuzzleSolver, Reference);

public:
	// Constants
	static const int EMPTY = 0;
	static const int BLACK = 1;
	static const int WHITE = 2;

private:
	// Internal board representation for fast operations
	struct BoardState {
		std::vector<int> cells;
		int size;

		BoardState() : size(0) {}
		BoardState(int s) : size(s), cells(s * s, EMPTY) {}
		BoardState(const BoardState &other) : size(other.size), cells(other.cells) {}

		inline int get(int x, int y) const {
			return cells[y * size + x];
		}

		inline void set(int x, int y, int value) {
			cells[y * size + x] = value;
		}

		inline bool in_bounds(int x, int y) const {
			return x >= 0 && x < size && y >= 0 && y < size;
		}

		String to_hash() const {
			String hash = "";
			for (size_t i = 0; i < cells.size(); i++) {
				hash += String::num_int64(cells[i]);
			}
			return hash;
		}

		bool operator==(const BoardState &other) const {
			return cells == other.cells;
		}
	};

	// Solution result structure
	struct SolutionResult {
		std::vector<std::pair<int, int>> player_moves;
		std::vector<std::pair<int, int>> opponent_moves;
		bool found;

		SolutionResult() : found(false) {}
	};

	// Position structure
	struct Position {
		int x, y;
		Position(int _x = -1, int _y = -1) : x(_x), y(_y) {}
	};

	// Direction offsets for neighbor checking
	static const int dx[4];
	static const int dy[4];

	// Internal helper methods
	bool group_has_liberties(const BoardState &board, int x, int y, std::vector<bool> &visited) const;
	void remove_group(BoardState &board, int x, int y, int color);
	void get_group_positions(const BoardState &board, int x, int y, int color,
							 std::vector<std::pair<int, int>> &positions, std::vector<bool> &visited) const;
	int count_captures(BoardState &board, int x, int y, int player_color);
	bool simulate_move(const BoardState &board, int x, int y, int color,
					   BoardState &result, const BoardState *previous = nullptr) const;
	bool target_stones_captured(const BoardState &board, const std::vector<std::pair<int, int>> &targets) const;
	bool no_opponent_stones(const BoardState &board, int player_color) const;
	bool check_goal(const BoardState &board, const std::vector<std::pair<int, int>> &targets, int player_color) const;

	// Solver methods
	SolutionResult find_winnable_solution(const BoardState &initial_board, int player_color,
										  const std::vector<std::pair<int, int>> &targets, int max_moves);
	SolutionResult search_winnable_path(const BoardState &board, int player_color, int opponent_color,
										const std::vector<std::pair<int, int>> &targets,
										std::vector<std::pair<int, int>> player_moves,
										std::vector<std::pair<int, int>> opponent_moves,
										int max_depth);
	SolutionResult find_opponent_response(const BoardState &board, int player_color, int opponent_color,
										  const std::vector<std::pair<int, int>> &targets,
										  std::vector<std::pair<int, int>> player_moves,
										  std::vector<std::pair<int, int>> opponent_moves,
										  int max_depth);
	int score_defensive_move(const BoardState &before, const BoardState &after, int x, int y,
							 int defender_color, int attacker_color) const;
	int count_total_liberties(const BoardState &board, int color) const;
	int count_group_liberties(const BoardState &board, int x, int y) const;

	// Game tree builder
	void build_game_tree_internal(const BoardState &initial_board,
								  const std::vector<std::pair<int, int>> &targets,
								  const std::vector<std::pair<int, int>> &solution_player_moves,
								  const std::vector<std::pair<int, int>> &solution_opponent_moves,
								  Dictionary &move_database,
								  int max_depth);
	std::pair<int, int> find_best_opponent_move(const BoardState &board,
												const std::vector<std::pair<int, int>> &legal_moves,
												const std::vector<std::pair<int, int>> &targets) const;

	// Conversion helpers
	BoardState array_to_board(const Array &board_2d) const;
	Array board_to_array(const BoardState &board) const;
	std::vector<std::pair<int, int>> array_to_positions(const Array &positions) const;
	Array positions_to_array(const std::vector<std::pair<int, int>> &positions) const;

protected:
	static void _bind_methods();

public:
	// Main API exposed to GDScript
	// Returns: Dictionary with "player_moves", "opponent_moves" arrays
	Dictionary solve_puzzle(const Array &board_2d, int player_color, const Array &target_stones, int max_moves = 6);

	// Build complete game tree with opponent responses
	// Returns: Dictionary mapping board_hash -> [x, y] opponent response
	Dictionary build_game_tree(const Array &board_2d, const Array &target_stones,
							   const Array &solution_player_moves, const Array &solution_opponent_moves,
							   int max_depth = 8);

	// Utility: Simulate a single move
	// Returns: new board state as 2D array, or empty array if invalid
	Array simulate_single_move(const Array &board_2d, int x, int y, int color);

	// Utility: Check if goal is achieved
	bool is_goal_achieved(const Array &board_2d, int player_color, const Array &target_stones);

	PuzzleSolver();
	~PuzzleSolver();
};

#endif // PUZZLE_SOLVER_H
