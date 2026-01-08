#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"

typedef enum {
	// no error occured
	HEXVIEW_NO_ERROR = 0,

	// file operation errors
	HEXVIEW_ERR_INVALID_INPUT,
	HEXVIEW_ERR_FILE_NOT_FOUND,
	HEXVIEW_ERR_NO_PERMISSION,
	HEXVIEW_ERR_READ_FAIL,
	HEXVIEW_ERR_IO_FAIL,
} hv_status_t;

typedef struct {
	char *str;
	size_t length;
} hv_string;

// String
hv_string *hv_new_string(Arena *a, char *string);
hv_string *hv_str_concat(Arena *a, hv_string *str1, size_t n, ...);
hv_string *hv_str_char_concat(Arena *a, hv_string *str1, size_t n, ...);
char *hv_char_repeat(Arena *a, char c, size_t count);

// I/O
hv_status_t hv_open_file(const char *path, FILE **pF);
hv_status_t hv_file_size(size_t *size, FILE *f);
hv_status_t hv_file_read(Arena *a, uint8_t **dest, size_t count, FILE *f);

// Terminal
uint32_t hv_get_terminal_cols();
hv_status_t print_buffer(char **buf);

// Table
hv_status_t hv_format_as_table(Arena *a, uint8_t *buf, size_t size,
							   hv_string **dest, uint32_t terminal_cols);
hv_status_t hv_format_as_table_color(uint8_t *buf, char **dest, uint32_t cols);
