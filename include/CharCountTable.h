#ifndef CHAR_COUNT_TABLE_H
#define CHAR_COUNT_TABLE_H

#include <ctype.h>

#include "SolverCommon.h"

typedef struct CharCountTable {
	uint16_t counts[CHAR_COUNT_TABLE_LEN];
} CharCountTable;

CharCountTable* CharCountTableAddTo(CharCountTable* lhs, const CharCountTable* rhs);

bool CharCountTableCanFit(const CharCountTable* table, const CharCountTable* toFit);

bool CharCountTableEquals(const CharCountTable* lhs, const CharCountTable* rhs);

CharCountTable* ReadCharCounts(const char* str);

#endif	// CHAR_COUNT_TABLE_H
