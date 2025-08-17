#ifndef DICT_H
#define DICT_H

#include "CharCountTable.h"

typedef struct Word {
	char data[MAX_WORD_LEN];
	uint8_t len;
	CharCountTable charTable;
} Word;

DeclareArrayType(Word, WordArray);
DeclareArrayMethods(Word, WordArray);

WordArray* ReadDict(const char* path);

#endif	// DICT_H
