#pragma once

#include <stdarg.h>
#include <stdbool.h>
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
	HEXVIEW_ERR_BUFFER_TOO_SMALL,
	HEXVIEW_ERR_READ_FAIL,
	HEXVIEW_ERR_IO_FAIL,
} hv_status_t;

// String
char *hv_char_repeat(Arena *a, char c, size_t count);

// I/O
hv_status_t hv_open_file(const char *path, FILE **pF);
hv_status_t hv_file_size(size_t *size, FILE *f);
hv_status_t hv_file_set_pos_start(FILE *f);
long hv_file_get_pos(FILE *f);
hv_status_t hv_file_read(uint8_t *dest, size_t dest_size, FILE *f,
						 size_t *out_read);

// Terminal
uint32_t hv_get_terminal_cols();

// Table
hv_status_t hv_format_as_table(uint8_t *buf, size_t buf_size, char *out,
							   size_t out_size, uint32_t terminal_cols,
							   bool include_table_header,
							   uint32_t *cur_byte_range);
hv_status_t hv_format_as_table_color(uint8_t *buf, char **dest, uint32_t cols);
