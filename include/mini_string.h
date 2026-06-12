#pragma once

#include "arena.h"
#include <stddef.h>

typedef struct {
	size_t length;
	size_t capacity;
	char *string;
} MiniString;

typedef enum {
	MS_SUCCESS = 0,
	MS_ERR_NULL_ARGUMENT,
	MS_ERR_OUT_OF_MEMORY,
	MS_ERR_INVALID_SLICE_RANGE,
	MS_ERR_STR_CAP_EXCEEDED,
} MS_Result;

MiniString MS_new_string(Arena *a, const char *source);

MiniString MS_new_string_cap(Arena *a, size_t capacity);

MS_Result MS_put_string(MiniString *s, const char *source);

MS_Result MS_append_cstr(MiniString *s, const char *source);

MS_Result MS_append_char(MiniString *s, char c);

MiniString MS_concat_string(Arena *a, const MiniString *str1,
							const MiniString *str2);

MiniString MS_new_string_format(Arena *a, const char *restrict format, ...);

MS_Result MS_string_slice(MiniString *out, const MiniString *s, size_t start,
						  size_t finish);

char *MS_to_cstr(Arena *a, MiniString slice);
