#include "main.h"

#include <ctype.h>

#define MAX_WORD_LEN 63

#define CHAR_COUNT_TABLE_LEN 0x100

typedef struct CharCountTable {
	uint16_t counts[CHAR_COUNT_TABLE_LEN];
} CharCountTable;

CharCountTable* CharCountTableAddTo(CharCountTable* lhs, const CharCountTable* rhs) {
	for (size_t i = 0; i < CHAR_COUNT_TABLE_LEN; i++) {
		lhs->counts[i] += rhs->counts[i];
	}

	return lhs;
}

bool CharCountTableCanFit(const CharCountTable* table, const CharCountTable* toFit) {
	for (size_t i = 0; i < CHAR_COUNT_TABLE_LEN; i++) {
		if (toFit->counts[i] > table->counts[i]) return false;
	}

	return true;
}

bool CharCountTableEquals(const CharCountTable* lhs, const CharCountTable* rhs) {
	for (size_t i = 0; i < CHAR_COUNT_TABLE_LEN; i++) {
		if (rhs->counts[i] != lhs->counts[i]) return false;
	}

	return true;
}

typedef struct Word {
	char data[MAX_WORD_LEN];
	uint8_t len;
	CharCountTable charTable;
} Word;

DeclareArrayType(Word, WordArray);
DeclareArrayMethods(Word, WordArray);
DefineArrayMethods(Word, WordArray);

WordArray* ReadDict(const char* path) {
	WordArray* dict = WordArrayCreate(8);
	if (!dict) return NULL;

	FILE* file = TryOpenFile(path, "r+");
	if (!file) {
		WordArrayFree(dict);
		return NULL;
	}

	char* line = NULL;
	int64_t lineLen = 0;
	size_t lineCapa = 0;
	size_t lineNum = 0;

	while ((lineLen = LuFileGetLine(&line, &lineCapa, file)) != LU_FILE_ERROR) {
		// Removing included newline and/or CR
		{
			char* tmp = strchr(line, '\n');
			if (tmp) *tmp = '\0';

			tmp = strchr(line, '\r');
			if (tmp) *tmp = '\0';

			lineLen = strlen(line);
		}

		lineNum++;

		if (lineLen > (int64_t)MAX_WORD_LEN) {
			LogMessage("%s:%zu : skipping \"%s\" (WORD_TOO_LONG: max=%d, curr=%zu)\n", path, lineNum, line, MAX_WORD_LEN, lineLen);
			continue;
		}

		Word word = {0};
		memcpy(word.data, line, lineLen * sizeof(char));
		word.len = lineLen;

		const char* str = line;
		while (*str) word.charTable.counts[tolower(*str++)]++;

		if (!WordArrayPush(dict, word)) {
			WordArrayFree(dict);
			CloseFile(file);
			return NULL;
		}
	}

	CloseFile(file);
	return dict;
}

CharCountTable* ReadCharCounts(const char* str) {
	CharCountTable* table = NULL;
	if (!Calloc(table, 1, sizeof(CharCountTable))) return NULL;

	while (*str) {
		if (!isspace(*str)) table->counts[tolower(*str)]++;
		str++;
	}

	return table;
}

DeclareArrayType(size_t, SizeArray);
DeclareArrayMethods(size_t, SizeArray);
DefineArrayMethods(size_t, SizeArray);

SizeArray* SizeArrayClone(const SizeArray* arr) {
	SizeArray* clone = SizeArrayCreate(arr->capacity);
	if (!clone) return NULL;

	clone->size = arr->size;
	memcpy(clone->data, arr->data, sizeof(*arr->data) * arr->capacity);
	return clone;
}

SizeArray* SplitHints(const char* str) {
	SizeArray* arr = SizeArrayCreate(8);
	if (!arr) return NULL;

	while (*str) {
		size_t subLen = strspn(str, "0123456789");
		size_t skipLen = strcspn(str + subLen, "012345789");

		if (subLen > MAX_WORD_LEN) {
			LogMessage("Exiting program : \"%.*s\" (HINT_TOO_LONG: max=%d, curr=%zu)\n", (int)subLen, str, MAX_WORD_LEN, subLen);
			SizeArrayFree(arr);
			return NULL;
		}

		size_t hint = strtoull(str, NULL, 10);

		if (!SizeArrayPush(arr, hint)) {
			SizeArrayFree(arr);
			return NULL;
		}

		str += subLen + skipLen;
	}

	return arr;
}

bool EraseImpossibleWords(WordArray* dict, CharCountTable* inputTable, SizeArray* hints) {
	for (int i = dict->size - 1; i >= 0; i--) {
		const Word* currWord = dict->data + i;

		bool shouldErase = true;
		for (size_t j = 0; j < hints->size; j++) {
			if (currWord->len != hints->data[j]) continue;
			shouldErase = false;
			break;
		}

		for (size_t j = 0; j < CHAR_COUNT_TABLE_LEN; j++) {
			if (currWord->charTable.counts[j] <= inputTable->counts[j]) continue;
			shouldErase = true;
			break;
		}

		if (shouldErase) {
			if (!WordArrayErase(dict, i)) {
				WordArrayFree(dict);
				return false;
			}
		}
	}

	return (dict->size > 0);
}

typedef struct Guess {
	CharCountTable charTable;
	SizeArray* wordIndeces;
} Guess;

static int guessCount = 0;

void TestAllCombinations(Guess currGuess, WordArray* dict, CharCountTable* expected, SizeArray* hints, size_t hintIdx) {
	if (hintIdx >= hints->size) {
		if (CharCountTableEquals(&(currGuess.charTable), expected)) {
			printf("Guess #%d: ", guessCount++);
			for (size_t i = 0; i < hints->size; i++) {
				const Word* word = dict->data + currGuess.wordIndeces->data[i];
				printf("%.*s ", (int)word->len, word->data);
			}
			printf("\n");
		}
		return;
	}

	if (!CharCountTableCanFit(expected, &(currGuess.charTable))) return;

	for (size_t i = 0; i < dict->size; i++) {
		const Word* word = dict->data + i;
		if (word->len != hints->data[hintIdx]) continue;

		Guess nextGuess = {0};

		nextGuess.charTable = currGuess.charTable;
		CharCountTableAddTo(&(nextGuess.charTable), &(word->charTable));

		if (hintIdx) nextGuess.wordIndeces = SizeArrayClone(currGuess.wordIndeces);
		if (!hintIdx) nextGuess.wordIndeces = SizeArrayCreate(hints->size);
		if (!nextGuess.wordIndeces) break;

		nextGuess.wordIndeces->data[hintIdx] = i;

		TestAllCombinations(nextGuess, dict, expected, hints, hintIdx + 1);
		if (!hintIdx) SizeArrayFree(nextGuess.wordIndeces);
	}
}

int main(int argc, char** argv) {
	if (argc < 3) {
		LogMessage("Not enough args\n");
		LogMessage("Usage: %s input_string out_fmt\n", *argv);
		LogMessage("\tin_str: The words as you see them in-game in but double quotes (ex: \"avian green\")\n");
		LogMessage("\tout_fmt: A double quoted string with the number of characters per word with spaces (ex: \"5 5\")\n");
		LogMessage("Example usage: %s \"avian green\" \"5 5\"\n", *argv);
		exit(EXIT_FAILURE);
	}

	WordArray* dict = ReadDict("ressources/dict.txt");
	CharCountTable* input = ReadCharCounts(argv[1]);
	SizeArray* hints = SplitHints(argv[2]);

	EraseImpossibleWords(dict, input, hints);
	TestAllCombinations((Guess){0}, dict, input, hints, 0);

	WordArrayFree(dict);
	Free(input);
	SizeArrayFree(hints);
	return 0;
}
