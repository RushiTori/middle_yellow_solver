#include "SizeArray.h"

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

		size_t hint = strtoull(str, NULL, 10);

		if (hint > MAX_WORD_LEN) {
			LogMessage("Exiting program : \"%.*s\" (HINT_TOO_LONG: max=%d, curr=%zu)\n", (int)subLen, str, MAX_WORD_LEN, hint);
			SizeArrayFree(arr);
			return NULL;
		}

		if (!SizeArrayPush(arr, hint)) {
			SizeArrayFree(arr);
			return NULL;
		}

		str += subLen + skipLen;
	}

	return arr;
}
