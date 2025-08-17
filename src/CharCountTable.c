#include "CharCountTable.h"

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

CharCountTable* ReadCharCounts(const char* str) {
	CharCountTable* table = NULL;
	if (!Calloc(table, 1, sizeof(CharCountTable))) return NULL;

	while (*str) {
		if (!isspace(*str)) table->counts[tolower(*str)]++;
		str++;
	}

	return table;
}
