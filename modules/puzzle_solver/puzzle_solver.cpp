/**************************************************************************/
/*  puzzle_solver.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "puzzle_solver.h"
#include "core/print_string.h"

// Direction offsets for neighbor checking
const int PuzzleSolver::dx[4] = { 1, -1, 0, 0 };
const int PuzzleSolver::dy[4] = { 0, 0, 1, -1 };

PuzzleSolver::PuzzleSolver() {
}

PuzzleSolver::~PuzzleSolver() {
}

void PuzzleSolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("solve_puzzle", "board_2d", "player_color", "target_stones", "max_moves"),
						 &PuzzleSolver::solve_puzzle, DEFVAL(6));
	ClassDB::bind_method(D_METHOD("build_game_tree", "board_2d", "target_stones",
								  "solution_player_moves", "solution_opponent_moves", "max_depth"),
						 &PuzzleSolver::build_game_tree, DEFVAL(8));
	ClassDB::bind_method(D_METHOD("simulate_single_move", "board_2d", "x", "y", "color"),
						 &PuzzleSolver::simulate_single_move);
	ClassDB::bind_method(D_METHOD("is_goal_achieved", "board_2d", "player_color", "target_stones"),
						 &PuzzleSolver::is_goal_achieved);

	BIND_CONSTANT(EMPTY);
	BIND_CONSTANT(BLACK);
	BIND_CONSTANT(WHITE);
}

// ============ Conversion Helpers ============

PuzzleSolver::BoardState PuzzleSolver::array_to_board(const Array &board_2d) const {
	if (board_2d.size() == 0) {
		return BoardState(0);
	}

	int size = board_2d.size();
	BoardState board(size);

	for (int y = 0; y < size; y++) {
		Array row = board_2d[y];
		for (int x = 0; x < row.size(); x++) {
			board.set(x, y, row[x]);
		}
	}

	return board;
}

Array PuzzleSolver::board_to_array(const BoardState &board) const {
	Array result;
	for (int y = 0; y < board.size; y++) {
		Array row;
		for (int x = 0; x < board.size; x++) {
			row.push_back(board.get(x, y));
		}
		result.push_back(row);
	}
	return result;
}

std::vector<std::pair<int, int>> PuzzleSolver::array_to_positions(const Array &positions) const {
	std::vector<std::pair<int, int>> result;
	for (int i = 0; i < positions.size(); i++) {
		Array pos = positions[i];
		if (pos.size() >= 2) {
			result.push_back(std::make_pair((int)pos[0], (int)pos[1]));
		}
	}
	return result;
}

Array PuzzleSolver::positions_to_array(const std::vector<std::pair<int, int>> &positions) const {
	Array result;
	for (size_t i = 0; i < positions.size(); i++) {
		Array pos;
		pos.push_back(positions[i].first);
		pos.push_back(positions[i].second);
		result.push_back(pos);
	}
	return result;
}

// ============ Go Rules Implementation ============

bool PuzzleSolver::group_has_liberties(const BoardState &board, int x, int y, std::vector<bool> &visited) const {
	int idx = y * board.size + x;
	if (visited[idx]) {
		return false;
	}
	visited[idx] = true;

	int color = board.get(x, y);

	for (int d = 0; d < 4; d++) {
		int nx = x + dx[d];
		int ny = y + dy[d];
		if (board.in_bounds(nx, ny)) {
			int neighbor = board.get(nx, ny);
			if (neighbor == EMPTY) {
				return true; // Found a liberty
			} else if (neighbor == color) {
				if (group_has_liberties(board, nx, ny, visited)) {
					return true;
				}
			}
		}
	}

	return false;
}

void PuzzleSolver::get_group_positions(const BoardState &board, int x, int y, int color,
									   std::vector<std::pair<int, int>> &positions, std::vector<bool> &visited) const {
	int idx = y * board.size + x;
	if (visited[idx] || !board.in_bounds(x, y) || board.get(x, y) != color) {
		return;
	}
	visited[idx] = true;
	positions.push_back(std::make_pair(x, y));

	for (int d = 0; d < 4; d++) {
		get_group_positions(board, x + dx[d], y + dy[d], color, positions, visited);
	}
}

void PuzzleSolver::remove_group(BoardState &board, int x, int y, int color) {
	if (!board.in_bounds(x, y) || board.get(x, y) != color) {
		return;
	}
	board.set(x, y, EMPTY);

	for (int d = 0; d < 4; d++) {
		remove_group(board, x + dx[d], y + dy[d], color);
	}
}

int PuzzleSolver::count_captures(BoardState &board, int x, int y, int player_color) {
	int opponent_color = (player_color == BLACK) ? WHITE : BLACK;
	int captured = 0;

	for (int d = 0; d < 4; d++) {
		int nx = x + dx[d];
		int ny = y + dy[d];
		if (board.in_bounds(nx, ny) && board.get(nx, ny) == opponent_color) {
			std::vector<bool> visited(board.size * board.size, false);
			if (!group_has_liberties(board, nx, ny, visited)) {
				// Find all stones in group and remove
				std::vector<std::pair<int, int>> group;
				std::vector<bool> group_visited(board.size * board.size, false);
				get_group_positions(board, nx, ny, opponent_color, group, group_visited);
				captured += group.size();
				remove_group(board, nx, ny, opponent_color);
			}
		}
	}

	return captured;
}

bool PuzzleSolver::simulate_move(const BoardState &board, int x, int y, int color,
								 BoardState &result, const BoardState *previous) const {
	if (!board.in_bounds(x, y) || board.get(x, y) != EMPTY) {
		return false;
	}

	result = board;
	result.set(x, y, color);

	int opponent_color = (color == BLACK) ? WHITE : BLACK;
	int captured_count = 0;

	// Capture opponent stones first
	for (int d = 0; d < 4; d++) {
		int nx = x + dx[d];
		int ny = y + dy[d];
		if (result.in_bounds(nx, ny) && result.get(nx, ny) == opponent_color) {
			std::vector<bool> visited(result.size * result.size, false);
			if (!group_has_liberties(result, nx, ny, visited)) {
				// Count and remove the group
				std::vector<std::pair<int, int>> group;
				std::vector<bool> group_visited(result.size * result.size, false);
				get_group_positions(result, nx, ny, opponent_color, group, group_visited);
				captured_count += group.size();

				for (size_t i = 0; i < group.size(); i++) {
					result.set(group[i].first, group[i].second, EMPTY);
				}
			}
		}
	}

	// Check for suicide (no liberties after placing)
	std::vector<bool> suicide_check(result.size * result.size, false);
	if (!group_has_liberties(result, x, y, suicide_check)) {
		return false; // Suicide move
	}

	// Check for Ko rule violation
	if (captured_count == 1 && previous != nullptr && result == *previous) {
		return false; // Ko violation
	}

	return true;
}

bool PuzzleSolver::target_stones_captured(const BoardState &board, const std::vector<std::pair<int, int>> &targets) const {
	for (size_t i = 0; i < targets.size(); i++) {
		int x = targets[i].first;
		int y = targets[i].second;
		if (board.in_bounds(x, y) && board.get(x, y) == WHITE) {
			return false; // Target still exists
		}
	}
	return true;
}

bool PuzzleSolver::no_opponent_stones(const BoardState &board, int player_color) const {
	int opponent = (player_color == BLACK) ? WHITE : BLACK;
	for (int y = 0; y < board.size; y++) {
		for (int x = 0; x < board.size; x++) {
			if (board.get(x, y) == opponent) {
				return false;
			}
		}
	}
	return true;
}

bool PuzzleSolver::check_goal(const BoardState &board, const std::vector<std::pair<int, int>> &targets, int player_color) const {
	if (!targets.empty()) {
		return target_stones_captured(board, targets);
	} else {
		return no_opponent_stones(board, player_color);
	}
}

// ============ Solution Finding ============

PuzzleSolver::SolutionResult PuzzleSolver::find_winnable_solution(const BoardState &initial_board, int player_color,
																  const std::vector<std::pair<int, int>> &targets, int max_moves) {
	int opponent_color = (player_color == BLACK) ? WHITE : BLACK;

	// Iterative deepening - try to find shortest solution first
	for (int depth = 1; depth <= max_moves; depth++) {
		SolutionResult result = search_winnable_path(initial_board, player_color, opponent_color,
													 targets, std::vector<std::pair<int, int>>(),
													 std::vector<std::pair<int, int>>(), depth);
		if (result.found) {
			return result;
		}
	}

	return SolutionResult();
}

PuzzleSolver::SolutionResult PuzzleSolver::search_winnable_path(const BoardState &board, int player_color, int opponent_color,
																const std::vector<std::pair<int, int>> &targets,
																std::vector<std::pair<int, int>> player_moves,
																std::vector<std::pair<int, int>> opponent_moves,
																int max_depth) {
	if ((int)player_moves.size() >= max_depth) {
		return SolutionResult();
	}

	// Check if we've already won
	if (check_goal(board, targets, player_color)) {
		SolutionResult result;
		result.found = true;
		result.player_moves = player_moves;
		result.opponent_moves = opponent_moves;
		return result;
	}

	// Try all legal player moves
	for (int y = 0; y < board.size; y++) {
		for (int x = 0; x < board.size; x++) {
			if (board.get(x, y) != EMPTY) {
				continue;
			}

			BoardState after_player;
			if (!simulate_move(board, x, y, player_color, after_player)) {
				continue;
			}

			std::vector<std::pair<int, int>> new_player_moves = player_moves;
			new_player_moves.push_back(std::make_pair(x, y));

			// Check if this move wins immediately
			if (check_goal(after_player, targets, player_color)) {
				SolutionResult result;
				result.found = true;
				result.player_moves = new_player_moves;
				result.opponent_moves = opponent_moves;
				return result;
			}

			// Find opponent response that still allows player to win
			SolutionResult response_result = find_opponent_response(after_player, player_color, opponent_color,
																	targets, new_player_moves, opponent_moves, max_depth);
			if (response_result.found) {
				return response_result;
			}
		}
	}

	return SolutionResult();
}

PuzzleSolver::SolutionResult PuzzleSolver::find_opponent_response(const BoardState &board, int player_color, int opponent_color,
																  const std::vector<std::pair<int, int>> &targets,
																  std::vector<std::pair<int, int>> player_moves,
																  std::vector<std::pair<int, int>> opponent_moves,
																  int max_depth) {
	// Collect all legal opponent moves with scores
	struct ScoredMove {
		int x, y;
		BoardState result;
		int score;
	};
	std::vector<ScoredMove> candidates;

	for (int y = 0; y < board.size; y++) {
		for (int x = 0; x < board.size; x++) {
			if (board.get(x, y) != EMPTY) {
				continue;
			}

			BoardState after_opponent;
			if (!simulate_move(board, x, y, opponent_color, after_opponent)) {
				continue;
			}

			int score = score_defensive_move(board, after_opponent, x, y, opponent_color, player_color);
			ScoredMove sm;
			sm.x = x;
			sm.y = y;
			sm.result = after_opponent;
			sm.score = score;
			candidates.push_back(sm);
		}
	}

	// Sort by score (descending - prefer more defensive moves)
	std::sort(candidates.begin(), candidates.end(), [](const ScoredMove &a, const ScoredMove &b) {
		return a.score > b.score;
	});

	// Also try "pass" (no response)
	{
		ScoredMove pass;
		pass.x = -1;
		pass.y = -1;
		pass.result = board;
		pass.score = -100;
		candidates.push_back(pass);
	}

	// For each candidate response, check if player can still win
	for (size_t i = 0; i < candidates.size(); i++) {
		const ScoredMove &candidate = candidates[i];

		std::vector<std::pair<int, int>> new_opponent_moves = opponent_moves;
		if (candidate.x >= 0) {
			new_opponent_moves.push_back(std::make_pair(candidate.x, candidate.y));
		}

		SolutionResult result = search_winnable_path(candidate.result, player_color, opponent_color,
													 targets, player_moves, new_opponent_moves, max_depth);
		if (result.found) {
			return result;
		}
	}

	return SolutionResult();
}

int PuzzleSolver::score_defensive_move(const BoardState &before, const BoardState &after, int x, int y,
									   int defender_color, int attacker_color) const {
	int score = 0;

	// Check adjacency to own and enemy stones
	int adjacent_own = 0;
	int adjacent_enemy = 0;

	for (int d = 0; d < 4; d++) {
		int nx = x + dx[d];
		int ny = y + dy[d];
		if (before.in_bounds(nx, ny)) {
			int neighbor = before.get(nx, ny);
			if (neighbor == defender_color) {
				adjacent_own++;
			} else if (neighbor == attacker_color) {
				adjacent_enemy++;
			}
		}
	}

	// Prefer connecting to own stones
	score += adjacent_own * 30;
	// Slight preference for moves near enemy (defensive)
	score += adjacent_enemy * 10;

	// Check liberties after move
	int our_liberties = count_total_liberties(after, defender_color);
	score += our_liberties * 5;

	// Position bias for determinism
	score += (x + y * 3) % 5;

	return score;
}

int PuzzleSolver::count_total_liberties(const BoardState &board, int color) const {
	int total = 0;
	std::vector<bool> counted(board.size * board.size, false);

	for (int y = 0; y < board.size; y++) {
		for (int x = 0; x < board.size; x++) {
			if (board.get(x, y) == color) {
				int idx = y * board.size + x;
				if (!counted[idx]) {
					// Count liberties for this group
					std::unordered_set<int> liberties;
					std::vector<bool> visited(board.size * board.size, false);

					// Find all stones in group and their liberties
					std::vector<std::pair<int, int>> stack;
					stack.push_back(std::make_pair(x, y));

					while (!stack.empty()) {
						auto pos = stack.back();
						stack.pop_back();

						int px = pos.first;
						int py = pos.second;
						int pidx = py * board.size + px;

						if (visited[pidx]) continue;
						if (!board.in_bounds(px, py)) continue;
						if (board.get(px, py) != color) continue;

						visited[pidx] = true;
						counted[pidx] = true;

						for (int d = 0; d < 4; d++) {
							int nx = px + dx[d];
							int ny = py + dy[d];
							if (board.in_bounds(nx, ny)) {
								int nval = board.get(nx, ny);
								if (nval == EMPTY) {
									liberties.insert(ny * board.size + nx);
								} else if (nval == color) {
									stack.push_back(std::make_pair(nx, ny));
								}
							}
						}
					}

					total += liberties.size();
				}
			}
		}
	}

	return total;
}

int PuzzleSolver::count_group_liberties(const BoardState &board, int x, int y) const {
	int color = board.get(x, y);
	if (color == EMPTY) return 0;

	std::unordered_set<int> liberties;
	std::vector<bool> visited(board.size * board.size, false);
	std::vector<std::pair<int, int>> stack;
	stack.push_back(std::make_pair(x, y));

	while (!stack.empty()) {
		auto pos = stack.back();
		stack.pop_back();

		int px = pos.first;
		int py = pos.second;
		int pidx = py * board.size + px;

		if (visited[pidx]) continue;
		if (!board.in_bounds(px, py)) continue;
		if (board.get(px, py) != color) continue;

		visited[pidx] = true;

		for (int d = 0; d < 4; d++) {
			int nx = px + dx[d];
			int ny = py + dy[d];
			if (board.in_bounds(nx, ny)) {
				int nval = board.get(nx, ny);
				if (nval == EMPTY) {
					liberties.insert(ny * board.size + nx);
				} else if (nval == color) {
					stack.push_back(std::make_pair(nx, ny));
				}
			}
		}
	}

	return liberties.size();
}

// ============ Game Tree Building ============

void PuzzleSolver::build_game_tree_internal(const BoardState &initial_board,
											const std::vector<std::pair<int, int>> &targets,
											const std::vector<std::pair<int, int>> &solution_player_moves,
											const std::vector<std::pair<int, int>> &solution_opponent_moves,
											Dictionary &move_database,
											int max_depth) {
	int player_color = BLACK;
	int opponent_color = WHITE;

	// Step 1: Populate solution path responses first
	{
		BoardState board = initial_board;
		BoardState previous;
		bool has_previous = false;

		for (size_t i = 0; i < solution_player_moves.size(); i++) {
			int px = solution_player_moves[i].first;
			int py = solution_player_moves[i].second;

			BoardState prev = board;
			BoardState after_player;
			if (!simulate_move(board, px, py, player_color, after_player, has_previous ? &previous : nullptr)) {
				print_error("Invalid player move in solution path at step " + String::num_int64(i));
				return;
			}

			String board_hash = after_player.to_hash();
			previous = prev;
			has_previous = true;

			// Store opponent response for this board state
			if (i < solution_opponent_moves.size()) {
				int ox = solution_opponent_moves[i].first;
				int oy = solution_opponent_moves[i].second;
				if (ox >= 0) {
					Array response;
					response.push_back(ox);
					response.push_back(oy);
					move_database[board_hash] = response;

					// Simulate opponent response
					prev = after_player;
					BoardState after_opponent;
					if (!simulate_move(after_player, ox, oy, opponent_color, after_opponent, &previous)) {
						print_error("Invalid opponent move in solution path at step " + String::num_int64(i));
						return;
					}
					previous = prev;
					board = after_opponent;
				} else {
					board = after_player;
				}
			} else {
				board = after_player;
			}
		}
	}

	// Step 2: BFS to explore all reachable board states
	struct QueueItem {
		BoardState board;
		int depth;
		bool is_player_turn;
		BoardState previous;
		bool has_previous;
	};

	std::queue<QueueItem> queue;
	std::unordered_set<std::string> processed;

	QueueItem initial;
	initial.board = initial_board;
	initial.depth = 0;
	initial.is_player_turn = true;
	initial.has_previous = false;
	queue.push(initial);

	while (!queue.empty()) {
		QueueItem current = queue.front();
		queue.pop();

		if (current.depth >= max_depth) {
			continue;
		}

		String board_hash = current.board.to_hash();
		std::string state_key = board_hash.utf8().get_data();
		state_key += current.is_player_turn ? "_p" : "_o";

		if (processed.find(state_key) != processed.end()) {
			continue;
		}
		processed.insert(state_key);

		// Check if game is over
		if (check_goal(current.board, targets, player_color)) {
			continue;
		}

		int current_color = current.is_player_turn ? player_color : opponent_color;

		// Generate all legal moves
		std::vector<std::pair<int, int>> legal_moves;
		for (int y = 0; y < current.board.size; y++) {
			for (int x = 0; x < current.board.size; x++) {
				if (current.board.get(x, y) != EMPTY) {
					continue;
				}
				BoardState result;
				if (simulate_move(current.board, x, y, current_color, result,
								  current.has_previous ? &current.previous : nullptr)) {
					legal_moves.push_back(std::make_pair(x, y));
				}
			}
		}

		if (legal_moves.empty()) {
			// No legal moves - opponent must pass (or player has no moves)
			if (!current.is_player_turn) {
				// Store pass in database if not already there
				if (!move_database.has(board_hash)) {
					Array response;
					response.push_back(-1);
					response.push_back(-1);
					move_database[board_hash] = response;
				}
				
				// Queue same position as player's turn (opponent passed)
				QueueItem next;
				next.board = current.board;
				next.depth = current.depth + 1;
				next.is_player_turn = true;
				next.previous = current.previous;
				next.has_previous = current.has_previous;
				queue.push(next);
			}
			continue;
		}

		if (!current.is_player_turn) {
			// Opponent's turn - find response if not already in database
			if (!move_database.has(board_hash)) {
				auto best = find_best_opponent_move(current.board, legal_moves, targets);
				if (best.first >= 0) {
					Array response;
					response.push_back(best.first);
					response.push_back(best.second);
					move_database[board_hash] = response;
				}
			}

			// Queue resulting position (player's turn next)
			if (move_database.has(board_hash)) {
				Array resp = move_database[board_hash];
				int rx = resp[0];
				int ry = resp[1];
				if (rx >= 0) {
					BoardState next_board;
					if (simulate_move(current.board, rx, ry, opponent_color, next_board,
									  current.has_previous ? &current.previous : nullptr)) {
						QueueItem next;
						next.board = next_board;
						next.depth = current.depth + 1;
						next.is_player_turn = true;
						next.previous = current.board;
						next.has_previous = true;
						queue.push(next);
					}
				}
			}
		} else {
			// Player's turn - explore all possible moves
			for (size_t i = 0; i < legal_moves.size(); i++) {
				int mx = legal_moves[i].first;
				int my = legal_moves[i].second;

				BoardState next_board;
				if (simulate_move(current.board, mx, my, player_color, next_board,
								  current.has_previous ? &current.previous : nullptr)) {
					QueueItem next;
					next.board = next_board;
					next.depth = current.depth + 1;
					next.is_player_turn = false;
					next.previous = current.board;
					next.has_previous = true;
					queue.push(next);
				}
			}
		}
	}
}

std::pair<int, int> PuzzleSolver::find_best_opponent_move(const BoardState &board,
														  const std::vector<std::pair<int, int>> &legal_moves,
														  const std::vector<std::pair<int, int>> &targets) const {
	if (legal_moves.empty()) {
		return std::make_pair(-1, -1);
	}

	int player_color = BLACK;
	int opponent_color = WHITE;

	int best_x = -1, best_y = -1;
	int best_score = -99999;

	for (size_t i = 0; i < legal_moves.size(); i++) {
		int x = legal_moves[i].first;
		int y = legal_moves[i].second;

		BoardState resulting_board;
		if (!simulate_move(board, x, y, opponent_color, resulting_board)) {
			continue;
		}

		int score = 0;

		// Prefer moves that keep our stones alive
		int our_liberties = count_total_liberties(resulting_board, opponent_color);
		score += our_liberties * 10;

		// Prefer moves that reduce opponent liberties
		int their_liberties = count_total_liberties(resulting_board, player_color);
		score -= their_liberties * 5;

		// Deterministic tiebreaker
		score += (x + y * 7) % 3;

		if (score > best_score) {
			best_score = score;
			best_x = x;
			best_y = y;
		}
	}

	if (best_x < 0 && !legal_moves.empty()) {
		best_x = legal_moves[0].first;
		best_y = legal_moves[0].second;
	}

	return std::make_pair(best_x, best_y);
}

// ============ Public API ============

Dictionary PuzzleSolver::solve_puzzle(const Array &board_2d, int player_color, const Array &target_stones, int max_moves) {
	Dictionary result;

	BoardState board = array_to_board(board_2d);
	std::vector<std::pair<int, int>> targets = array_to_positions(target_stones);

	SolutionResult solution = find_winnable_solution(board, player_color, targets, max_moves);

	result["found"] = solution.found;
	result["player_moves"] = positions_to_array(solution.player_moves);
	result["opponent_moves"] = positions_to_array(solution.opponent_moves);

	return result;
}

Dictionary PuzzleSolver::build_game_tree(const Array &board_2d, const Array &target_stones,
										 const Array &solution_player_moves, const Array &solution_opponent_moves,
										 int max_depth) {
	Dictionary move_database;

	BoardState board = array_to_board(board_2d);
	std::vector<std::pair<int, int>> targets = array_to_positions(target_stones);
	std::vector<std::pair<int, int>> player_moves = array_to_positions(solution_player_moves);
	std::vector<std::pair<int, int>> opponent_moves = array_to_positions(solution_opponent_moves);

	build_game_tree_internal(board, targets, player_moves, opponent_moves, move_database, max_depth);

	return move_database;
}

Array PuzzleSolver::simulate_single_move(const Array &board_2d, int x, int y, int color) {
	BoardState board = array_to_board(board_2d);
	BoardState result;

	if (simulate_move(board, x, y, color, result)) {
		return board_to_array(result);
	}

	return Array();
}

bool PuzzleSolver::is_goal_achieved(const Array &board_2d, int player_color, const Array &target_stones) {
	BoardState board = array_to_board(board_2d);
	std::vector<std::pair<int, int>> targets = array_to_positions(target_stones);

	return check_goal(board, targets, player_color);
}
