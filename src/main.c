#include "main.h"

int main(int argc, char** argv) {
	Solver solver = {0};
	if (!SolverInit(&solver, argc, argv)) exit(EXIT_FAILURE);

	SolverPrintAllGuesses(&solver);

	SolverFree(&solver);

	exit(EXIT_SUCCESS);
}
