/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "register_types.h"
#include "core/class_db.h"
#include "puzzle_solver.h"

void register_puzzle_solver_types() {
	ClassDB::register_class<PuzzleSolver>();
}

void unregister_puzzle_solver_types() {
}
