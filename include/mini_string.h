#pragma once

#include "arena.h"
#include <string.h>

typedef struct {
	size_t capacity;
	size_t offset;
	char string[];
} MiniString;

MiniString *new_string(Arena *a, char *source);
MiniString *concat_string(Arena *a, MiniString *str1, MiniString *str2);
MiniString *new_string_format(Arena *a, const char *restrict format);
