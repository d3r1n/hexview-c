#include "utils.h"
#include "arena.h"
#include "config.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CLAMP(x, MIN, MAX) fmax(MIN, fmin(x, MAX))

hv_string *hv_new_string(Arena *a, char *string) {
	size_t length = strlen(string);
	hv_string *res = (hv_string *)arena_alloc(a, sizeof(hv_string));
	assert(res != NULL);

	res->length = length;
	res->str = (char *)arena_alloc(a, length);
	assert(res->str != NULL);
	memcpy(res->str, string, length);

	return res;
}

hv_string *hv_str_concat(Arena *a, hv_string *str1, size_t n, ...) {
	va_list args;

	size_t total_length = str1->length;
	va_start(args, n);
	for (size_t i = 0; i < n; i++) {
		hv_string *s = va_arg(args, hv_string *);
		if (s)
			total_length += s->length;
	}
	va_end(args); // MUST end the first pass

	hv_string *res = (hv_string *)arena_alloc(a, sizeof(hv_string));
	res->length = total_length;
	res->str = (char *)arena_alloc(a, total_length + 1);

	memcpy(res->str, str1->str, str1->length);

	// Set current_pos to the end of str1
	char *current_pos = res->str + str1->length;

	va_start(args, n);
	for (size_t i = 0; i < n; i++) {
		hv_string *s = va_arg(args, hv_string *);
		if (s && s->str) {
			memcpy(current_pos, s->str,
				   s->length);		  // Copy directly TO current_pos
			current_pos += s->length; // Advance the pointer
		}
	}
	va_end(args);

	return res;
}

hv_string *hv_str_char_concat(Arena *a, hv_string *str1, size_t n, ...) {
	va_list args;

	size_t total_length = str1->length;
	va_start(args, n);
	for (size_t i = 0; i < n; i++) {
		char *s = va_arg(args, char *);
		if (s)
			total_length += strlen(s);
	}
	va_end(args); // Clean up first pass

	hv_string *res = (hv_string *)arena_alloc(a, sizeof(hv_string));
	res->length = total_length;
	res->str = (char *)arena_alloc(a, total_length + 1);

	memcpy(res->str, str1->str, str1->length);
	char *current_pos = res->str + str1->length;

	va_start(args, n); // Reset the cursor to the beginning
	for (size_t i = 0; i < n; i++) {
		char *s = va_arg(args, char *);
		if (s) {
			size_t l = strlen(s);
			memcpy(current_pos, s, l); // Copy to current position
			current_pos += l;		   // Move position forward
		}
	}
	va_end(args);

	return res;
}

char *hv_str_center(Arena *a, char *text, uint32_t width) {
	uint32_t text_length = (uint32_t)strlen(text);

	// If width is too small, we must truncate to keep table alignment
	if (width <= text_length) {
		char *truncated = (char *)arena_alloc(a, width + 1);
		memcpy(truncated, text, width);
		truncated[width] = '\0';
		return truncated;
	}

	// Allocate width + 1 for the null terminator
	char *res = (char *)arena_alloc(a, width + 1);

	uint32_t total_padding = width - text_length;
	uint32_t left_padding = total_padding / 2;
	uint32_t right_padding = total_padding - left_padding;

	// Write the padding and text
	memset(res, ' ', left_padding);
	memcpy(res + left_padding, text, text_length);
	memset(res + left_padding + text_length, ' ', right_padding);

	// CRITICAL: Terminate the string
	res[width] = '\0';

	return res;
}

char *hv_char_repeat(Arena *a, char c, size_t count) {
	char *res = (char *)arena_alloc(a, count + 1);
	assert(res != NULL);

	memset(res, c, count);
	res[count] = '\0';
	return res;
}

static const char *MODE_READ_BINARY = "rb";

hv_status_t hv_open_file(const char *path, FILE **pF) {
	FILE *f = fopen(path, MODE_READ_BINARY);

	// check if file opened successfully
	if (!f) {
		switch (errno) {
		case EACCES:
			return HEXVIEW_ERR_NO_PERMISSION;
		case ENOENT:
			return HEXVIEW_ERR_FILE_NOT_FOUND;
		default:
			return HEXVIEW_ERR_IO_FAIL;
		}
	}

	*pF = f;
	return HEXVIEW_NO_ERROR;
}

hv_status_t hv_file_size(size_t *size, FILE *f) {
	int res = fseek(f, 0, SEEK_END); // seek to end

	if (res != 0)
		return HEXVIEW_ERR_IO_FAIL;

	long file_size = ftell(f); // get the current pos
	if (file_size == -1L)
		return HEXVIEW_ERR_IO_FAIL;

	rewind(f); // go back to beginning

	*size = (size_t)file_size;
	return HEXVIEW_NO_ERROR;
}

hv_status_t hv_file_read(Arena *a, uint8_t **dest, size_t count, FILE *f) {
	uint8_t *contents = arena_alloc(a, sizeof(uint8_t) * count);
	assert(contents != NULL);

	size_t num_obj_read = fread(contents, sizeof(*dest), count, f);

	// if we read less than we requested and its not end of file,
	// and theres an error that means I/O operation failed.
	if (num_obj_read < count && !feof(f) && ferror(f)) {
		// TODO: find a way to correctly handle the error in the caller function
		// clearerr(f); // clear the error for reading once again
		return HEXVIEW_ERR_READ_FAIL;
	}

	// put the values into specified variables
	*dest = contents;
	return HEXVIEW_NO_ERROR;
}

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

uint32_t hv_get_terminal_cols() {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
		return csbi.srWindow.Right - csbi.srWindow.Left + 1;
	} else {
		return MIN_COLUMNS
	}
#else
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
		return w.ws_col;
	} else {
		return MIN_COLUMNS;
	}
#endif
}

char *hv_binary_to_ascii(Arena *a, uint8_t *buf, size_t size) {
	char *ascii = (char *)arena_alloc(a, size);

	for (size_t i = 0; i < size; i++) {
		// if the binary is anything but basic ascii
		// replace it with " " to make text more readable
		if (buf[i] > 127) {
			ascii[i] = ' ';
		} else {
			ascii[i] = (char)buf[i];
		}
	}

	return ascii;
}

hv_status_t hv_format_as_table(Arena *a, uint8_t *buf, size_t size,
							   hv_string **dest, uint32_t terminal_cols) {
	if (!buf || size == 0 || terminal_cols < 20)
		return HEXVIEW_ERR_INVALID_INPUT;

	// 1. Apportion terminal width (percentages)
	// We reserve ~10 chars for separators: "| " (x3) and " |" (x1) and spaces
	uint32_t available_width = terminal_cols - 10;

	uint32_t w_range = (uint32_t)(available_width * 0.15f);
	uint32_t w_ascii = (uint32_t)(available_width * 0.25f);
	uint32_t w_hex = (uint32_t)(available_width * 0.60f);
	w_range = CLAMP(w_range, BYTE_COLS_MIN, BYTE_COLS_MAX);
	w_ascii = CLAMP(w_range, ASCII_COLS_MIN, ASCII_COLS_MAX);
	w_hex = CLAMP(w_range, HEX_COLS_MIN, HEX_COLS_MAX);

	// each byte "XX " (3chars)
	uint32_t bytes_per_row = fmax((uint32_t)(w_hex / 3), 1);

	// Adjust w_hex to be exactly what we use to avoid alignment gaps
	w_hex = bytes_per_row * 3;

	uint32_t num_rows = (uint32_t)((size + bytes_per_row - 1) / bytes_per_row);

	// 3. Allocate the output buffer
	// Estimate: rows * terminal_cols + header space
	Arena *temp_arena = arena_init((num_rows + 3) * terminal_cols * 2);
	char *result =
		(char *)arena_alloc(temp_arena, (num_rows + 3) * terminal_cols);
	uint32_t offset = 0;

	// 4. Header
	offset += sprintf(result + offset, "| %-*s | %-*s | %-*s |\n", w_range,
					  "RANGE", w_hex, "DATA (HEX)", w_ascii, "ASCII");

	// Separator line (---)
	offset += sprintf(
		result + offset, "%s\n",
		hv_char_repeat(temp_arena, '-', w_range + w_hex + w_ascii + 10));

	// 5. Build Rows
	for (uint32_t i = 0; i < num_rows; i++) {
		uint32_t start = i * bytes_per_row;
		uint32_t end = start + bytes_per_row;
		if (end > size)
			end = (uint32_t)size;

		// Col 1: Range
		char range_buf[32];
		sprintf(range_buf, "%u - %u", start, end - 1);
		offset += sprintf(result + offset, "| %-*s | ", w_range, range_buf);

		// Col 2: Hex
		uint32_t hex_start_offset = offset;
		for (uint32_t j = start; j < end; j++) {
			offset += sprintf(result + offset, "%02X ", buf[j]);
		}
		// Pad Hex column if row is short
		while ((offset - hex_start_offset) < w_hex) {
			result[offset++] = ' ';
		}
		offset += sprintf(result + offset, " | ");

		// Col 3: ASCII
		uint32_t ascii_start_offset = offset;
		for (uint32_t j = start; j < end; j++) {
			uint8_t c = buf[j];
			result[offset++] = (c >= 32 && c <= 126) ? (char)c : '.';
		}
		// Pad ASCII column if row is short
		while ((offset - ascii_start_offset) < w_ascii) {
			result[offset++] = ' ';
		}
		offset += sprintf(result + offset, " |\n");
	}

	*dest = hv_new_string(a, result);

	arena_free(temp_arena);
	return HEXVIEW_NO_ERROR;
}
