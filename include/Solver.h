#ifndef SOLVER_H
#define SOLVER_H

#include "Dict.h"
#include "SizeArray.h"

typedef struct Solver {
	size_t guessCount;
	WordArray* dict;
	CharCountTable* input;
	SizeArray* hints;
} Solver;

bool SolverInit(Solver* solver, int argc, char** argv);

void SolverFree(Solver* solver);

void SolverPrintAllGuesses(Solver* solver);

#endif	// SOLVER_H
