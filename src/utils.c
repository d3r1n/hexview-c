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
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

hv_status_t hv_file_read(uint8_t *dest, size_t dest_size, FILE *f,
						 size_t *out_read) {
	// number of objects read
	size_t n = fread(dest, sizeof(*dest), dest_size, f);

	// if we read less and there's an error
	// return error
	if (n < dest_size && ferror(f)) {
		return HEXVIEW_ERR_READ_FAIL;
	}

	// write into out_read how many objects we read
	if (out_read)
		*out_read = n;

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
	char *ascii = (char *)arena_alloc(a, size + 1);

	for (size_t i = 0; i < size; i++) {
		uint8_t c = buf[i];
		ascii[i] = (c >= 32 && c <= 126) ? (char)c : ' ';
	}

	ascii[size] = '\0';
	return ascii;
}

hv_status_t hv_format_as_table(uint8_t *buf, size_t buf_size, char *out,
							   size_t out_size, uint32_t terminal_cols,
							   bool include_tabel_header,
							   uint32_t *cur_byte_range) {
	if (!buf || !out || out_size == 0 || buf_size == 0 || terminal_cols < 20)
		return HEXVIEW_ERR_INVALID_INPUT;

	Arena *temp_arena = arena_init(1024 * 2);

	uint32_t available_width = terminal_cols - 10;

	uint32_t w_range = (uint32_t)(available_width * 0.15f);
	uint32_t w_ascii = (uint32_t)(available_width * 0.25f);
	uint32_t w_hex = (uint32_t)(available_width * 0.60f);

	w_range = CLAMP(w_range, BYTE_COLS_MIN, BYTE_COLS_MAX);
	w_ascii = CLAMP(w_ascii, ASCII_COLS_MIN, ASCII_COLS_MAX);
	w_hex = CLAMP(w_hex, HEX_COLS_MIN, HEX_COLS_MAX);

	uint32_t bytes_per_row = (w_hex / 3) ? (w_hex / 3) : 1;
	w_hex = bytes_per_row * 3;

	uint32_t num_rows =
		(uint32_t)((buf_size + bytes_per_row - 1) / bytes_per_row);

	size_t offset = 0;

// Helper macro for safe append
#define APPEND(fmt, ...)                                                       \
	do {                                                                       \
		int n = snprintf(out + offset, out_size - offset, fmt, ##__VA_ARGS__); \
		if (n < 0 || (size_t)n >= out_size - offset)                           \
			return HEXVIEW_ERR_BUFFER_TOO_SMALL;                               \
		offset += (size_t)n;                                                   \
	} while (0)

	if (include_tabel_header) {
		// header
		APPEND("| %-*s | %-*s | %-*s |\n", w_range, "RANGE", w_hex,
			   "DATA (HEX)", w_ascii, "ASCII");

		// seperator
		APPEND("%s\n",
			   hv_char_repeat(temp_arena, '-', w_range + w_hex + w_ascii + 10));
	}

	for (uint32_t i = 0; i < num_rows; i++) {
		uint32_t start = i * bytes_per_row + *cur_byte_range;
		uint32_t end = start + bytes_per_row + *cur_byte_range;
		if (end > buf_size)
			end = (uint32_t)buf_size;

		char range_buf[32];
		snprintf(range_buf, sizeof(range_buf), "%u - %u", start, end - 1);

		APPEND("| %-*s | ", w_range, range_buf);

		size_t hex_start = offset;
		for (uint32_t j = start; j < end; j++)
			APPEND("%02X ", buf[j]);

		while ((offset - hex_start) < w_hex) {
			if (offset + 1 >= out_size)
				return HEXVIEW_ERR_BUFFER_TOO_SMALL;
			out[offset++] = ' ';
		}

		APPEND(" | ");

		size_t ascii_start = offset;
		for (uint32_t j = start; j < end; j++) {
			uint8_t c = buf[j];
			if (offset + 1 >= out_size)
				return HEXVIEW_ERR_BUFFER_TOO_SMALL;
			out[offset++] = (c >= 32 && c <= 126) ? (char)c : '.';
		}

		while ((offset - ascii_start) < w_ascii) {
			if (offset + 1 >= out_size)
				return HEXVIEW_ERR_BUFFER_TOO_SMALL;
			out[offset++] = ' ';
		}

		APPEND(" |\n");
	}

	if (offset < out_size)
		out[offset] = '\0';

	*cur_byte_range += offset;

	arena_free(temp_arena);
	return HEXVIEW_NO_ERROR;
}
