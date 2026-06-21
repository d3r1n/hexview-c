#include "mini_string.h"
#include "arena.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

size_t MS_strlen(const char *restrict source) {
	char *a = (char *)source; // explicit conversion

	// while *a (char) != '\0'
	while (*a) {
		a++;
	}

	return a - source;
}

MiniString MS_new_string(Arena *a, const char *source) {
	MiniString s = {0};
	if (!a || !source)
		return s; // invalid input -> return empty ministring

	size_t length = MS_strlen(source);
	s.length = length;
	s.capacity = length;

	size_t size_str = sizeof(char) * (length + 1);
	s.string = (char *)arena_alloc(a, size_str);

	memcpy(s.string, source, length);
	s.string[length] = '\0';

	return s;
}

MiniString MS_new_string_cap(Arena *a, size_t capacity) {
	MiniString s = {0};
	if (!a)
		return s;

	// allocate capacity + 1 so there's always room for the null-terminator
	s.string = (char *)arena_alloc(a, sizeof(char) * (capacity + 1));
	s.length = 0;
	s.capacity = capacity;
	// keep null-terminated
	if (s.string)
		s.string[0] = '\0';

	return s;
}

MS_Result MS_put_string(MiniString *s, const char *source) {
	if (s == NULL || source == NULL)
		return MS_ERR_NULL_ARGUMENT;

	size_t source_len = MS_strlen(source);

	if (source_len > s->capacity) {
		return MS_ERR_STR_CAP_EXCEEDED;
	}

	// put the new string and ensure null-termination
	memcpy(s->string, source, source_len);
	s->string[source_len] = '\0';

	s->length = source_len;

	return MS_SUCCESS;
}

MS_Result MS_append_cstr(MiniString *s, const char *source) {
	if (s == NULL || source == NULL)
		return MS_ERR_NULL_ARGUMENT;

	size_t source_len = MS_strlen(source);

	if (source_len + s->length > s->capacity) {
		return MS_ERR_STR_CAP_EXCEEDED;
	}

	memcpy(s->string + s->length, source, source_len);
	s->length += source_len;
	// null-terminate
	s->string[s->length] = '\0';

	return MS_SUCCESS;
}

MS_Result MS_append_char(MiniString *s, char c) {
	if (s == NULL)
		return MS_ERR_NULL_ARGUMENT;

	if (s->length + 1 > s->capacity) {
		return MS_ERR_STR_CAP_EXCEEDED;
	}

	s->string[s->length] = c;
	s->length++;
	// null-terminate
	s->string[s->length] = '\0';

	return MS_SUCCESS;
}

MiniString MS_concat_string(Arena *a, const MiniString *str1,
                            const MiniString *str2) {
	MiniString s = {0};
	if (!a || !str1 || !str2)
		return s;

	s.length = str1->length + str2->length;
	s.capacity = s.length;

	s.string = arena_alloc(a, sizeof(char) * (s.length + 1));
	if (!s.string)
		return s;

	memcpy(s.string, str1->string, str1->length);
	memcpy(s.string + str1->length, str2->string, str2->length);

	s.string[s.length] = '\0';

	return s;
}

MiniString MS_new_string_format(Arena *a, const char *restrict format, ...) {
	MiniString s = {0};
	if (!a || !format)
		return s;

	va_list list;
	va_list list2;

	va_start(list, format);
	va_copy(list2, list);

	// returns the expected size of the string/buffer
	// as in number of bytes not including the null-terminator
	// NOTE: "list" variable is consumed
	size_t buffer_size = vsnprintf(NULL, 0, format, list);
	va_end(list); // dealloc consumed list

	// actual allocation of the string in the heap
	// with the null terminator
	char *buf = (char *)arena_alloc(a, buffer_size + 1);
	if (!buf) {
		va_end(list2);
		return s;
	}

	// put it into the buffer
	// NOTE: use the "list2" because "list" is consumed
	vsnprintf(buf, buffer_size + 1, format, list2);
	va_end(list2); // dealloc consumed list

	// create the ministring struct

	s.string = buf;
	s.length = buffer_size;
	s.capacity = s.length;

	return s;
}

// without allocating a new string, return a constrained view of the string as a
// slice
MS_Result MS_string_slice(MiniString *out, const MiniString *str, size_t start,
                          size_t finish) {
	if (str == NULL) {
		return MS_ERR_NULL_ARGUMENT;
	}

	bool start_bigger_length = start > str->length;
	bool finish_bigger_length = finish > str->length;
	bool slice_bigger_length = finish - start > str->length;

	if (start_bigger_length || finish_bigger_length || slice_bigger_length) {
		return MS_ERR_INVALID_SLICE_RANGE;
	}

	out->string = str->string + start;
	out->length = finish - start;

	return MS_SUCCESS;
}

// Returns a standard, null-terminated C-string
char *MS_to_cstr(Arena *a, MiniString slice) {
	// We add +1 directly to the size_bytes parameter of arena_alloc
	char *cstr = (char *)arena_alloc(a, slice.length + 1);
	if (!cstr)
		return NULL;

	if (slice.length > 0 && slice.string != NULL) {
		memcpy(cstr, slice.string, slice.length);
	}

	// add the null-term for correct c-string handling
	cstr[slice.length] = '\0';

	return cstr;
}
