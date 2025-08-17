#include "Solver.h"

static bool EraseImpossibleWords(Solver* solver) {
	for (int i = solver->dict->size - 1; i >= 0; i--) {
		const Word* currWord = solver->dict->data + i;

		bool shouldErase = true;
		for (size_t j = 0; j < solver->hints->size; j++) {
			if (currWord->len != solver->hints->data[j]) continue;
			shouldErase = false;
			break;
		}

		for (size_t j = 0; j < CHAR_COUNT_TABLE_LEN; j++) {
			if (currWord->charTable.counts[j] <= solver->input->counts[j]) continue;
			shouldErase = true;
			break;
		}

		if (shouldErase) {
			if (!WordArrayErase(solver->dict, i)) return false;
		}
	}

	return (solver->dict->size > 0);
}

bool SolverInit(Solver* solver, int argc, char** argv) {
	if (argc < 3) {
		LogMessage("Not enough args\n");
		LogMessage("Usage: %s input_string out_fmt\n", *argv);
		LogMessage("\tin_str: The words as you see them in-game in but double quotes (ex: \"avian green\")\n");
		LogMessage("\tout_fmt: A double quoted string with the number of characters per word with spaces (ex: \"5 5\")\n");
		LogMessage("Example usage: %s \"avian green\" \"5 5\"\n", *argv);
		return false;
	}

	solver->dict = ReadDict("ressources/dict.txt");
	solver->input = ReadCharCounts(argv[1]);
	solver->hints = SplitHints(argv[2]);

	if (!solver->dict || !solver->input || !solver->hints) {
		SolverFree(solver);
		return false;
	}

	if (!EraseImpossibleWords(solver)) {
		SolverFree(solver);
		return false;
	}

	return true;
}

void SolverFree(Solver* solver) {
	WordArrayFree(solver->dict);
	Free(solver->input);
	SizeArrayFree(solver->hints);
}

typedef struct Guess {
	CharCountTable charTable;
	SizeArray* wordIndeces;
} Guess;

static void TestAllCombinations(Guess currGuess, Solver* solver, size_t hintIdx) {
	if (hintIdx >= solver->hints->size) {
		if (CharCountTableEquals(&(currGuess.charTable), solver->input)) {
			printf("Guess #%zu: ", ++(solver->guessCount));
			for (size_t i = 0; i < solver->hints->size; i++) {
				const Word* word = solver->dict->data + currGuess.wordIndeces->data[i];
				printf("%.*s ", (int)word->len, word->data);
			}
			printf("\n");
		}
		return;
	}

	if (!CharCountTableCanFit(solver->input, &(currGuess.charTable))) return;

	for (size_t i = 0; i < solver->dict->size; i++) {
		const Word* word = solver->dict->data + i;
		if (word->len != solver->hints->data[hintIdx]) continue;

		Guess nextGuess = {0};

		nextGuess.charTable = currGuess.charTable;
		CharCountTableAddTo(&(nextGuess.charTable), &(word->charTable));

		if (hintIdx) nextGuess.wordIndeces = SizeArrayClone(currGuess.wordIndeces);
		if (!hintIdx) nextGuess.wordIndeces = SizeArrayCreate(solver->hints->size);
		if (!nextGuess.wordIndeces) break;

		nextGuess.wordIndeces->data[hintIdx] = i;

		TestAllCombinations(nextGuess, solver, hintIdx + 1);
		if (!hintIdx) SizeArrayFree(nextGuess.wordIndeces);
	}
}

void SolverPrintAllGuesses(Solver* solver) { TestAllCombinations((Guess){0}, solver, 0); }
