#ifndef SIZE_ARRAY_H
#define SIZE_ARRAY_H

#include "SolverCommon.h"

DeclareArrayType(size_t, SizeArray);
DeclareArrayMethods(size_t, SizeArray);

SizeArray* SizeArrayClone(const SizeArray* arr);

SizeArray* SplitHints(const char* str);

#endif	// SIZE_ARRAY_H
