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
	HV_SUCCESS = 0,

	// file operation errors
	HV_ERR_INVALID_INPUT,
	HV_ERR_FILE_NOT_FOUND,
	HV_ERR_NO_PERMISSION,
	HV_ERR_BUFFER_TOO_SMALL,
	HV_ERR_READ_FAIL,
	HV_ERR_IO_FAIL,
} hv_status_t;

// I/O
hv_status_t hv_open_file(const char *path, FILE **pF);
hv_status_t hv_file_size(size_t *size, FILE *f);
long hv_file_get_pos(FILE *f);
// Read into an existing buffer (more idiomatic than uint8_t**)
hv_status_t hv_file_read(uint8_t *dest, size_t dest_size, FILE *f,
                         uint32_t *out_read);

// Terminal
uint32_t hv_get_terminal_cols();

// Table (streaming output)
// Write the header to the provided stream
hv_status_t hv_display_header(Arena *a, FILE *dest, uint32_t w_range,
                              uint32_t w_hex, uint32_t w_ascii, bool use_color);

// Format buffer into rows and stream directly to 'dest'
hv_status_t hv_format_as_table(Arena *a, const uint8_t *buf, size_t buf_size,
                               FILE *dest, uint32_t w_range, uint32_t w_hex,
                               uint32_t bytes_per_row);

hv_status_t hv_format_as_table_color(Arena *a, const uint8_t *buf,
                                     size_t buf_size, FILE *dest,
                                     uint32_t w_range, uint32_t w_hex,
                                     uint32_t bytes_per_row);
