#include "Dict.h"

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
