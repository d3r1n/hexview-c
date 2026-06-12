#include "utils.h"
#include "arena.h"
#include "config.h"
#include "mini_string.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *hv_char_repeat(Arena *a, char c, size_t count) {
	if (!a || count == 0) return NULL;
	char *res = (char *)arena_alloc(a, count + 1);
	if (!res) return NULL;

	memset(res, c, count);
	res[count] = '\0';
	return res;
}

static const char *MODE_READ_BINARY = "rb";

hv_status_t hv_open_file(const char *path, FILE **pF) {
	if (!path || !pF) return HV_ERR_INVALID_INPUT;

	FILE *f = fopen(path, MODE_READ_BINARY);

	// check if file opened successfully
	if (!f) {
		switch (errno) {
		case EACCES:
			return HV_ERR_NO_PERMISSION;
		case ENOENT:
			return HV_ERR_FILE_NOT_FOUND;
		default:
			return HV_ERR_IO_FAIL;
		}
	}

	*pF = f;
	return HV_SUCCESS;
}

hv_status_t hv_file_size(size_t *size, FILE *f) {
	if (!size || !f) return HV_ERR_INVALID_INPUT;

	int res = fseek(f, 0, SEEK_END); // seek to end

	if (res != 0)
		return HV_ERR_IO_FAIL;

	long file_size = ftell(f); // get the current pos
	if (file_size == -1L)
		return HV_ERR_IO_FAIL;

	rewind(f); // go back to beginning

	*size = (size_t)file_size;
	return HV_SUCCESS;
}

hv_status_t hv_file_set_pos_start(FILE *f) {
	if (!f) return HV_ERR_INVALID_INPUT;
	if (fseek(f, 0, SEEK_SET) != 0) return HV_ERR_IO_FAIL;
	return HV_SUCCESS;
}

long hv_file_get_pos(FILE *f) {
	if (!f) return -1;
	long pos = ftell(f);
	return pos;
}

hv_status_t hv_file_read(uint8_t *dest, size_t dest_size, FILE *f,
                         uint32_t *out_read) {
	if (!dest || !f || dest_size == 0) return HV_ERR_INVALID_INPUT;

	// number of objects read
	size_t n = fread(dest, sizeof(*dest), dest_size, f);

	// if we read less and there's an error, return error
	if (n < dest_size && ferror(f)) {
		return HV_ERR_READ_FAIL;
	}

	if (out_read)
		*out_read = (uint32_t)n;

	return HV_SUCCESS;
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
		return MIN_COLUMNS;
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

/* Helper: convert binary buffer to a printable ministring (ascii-safe) */
inline MiniString hv_binary_to_string(Arena *a, const uint8_t *buf, size_t size) {
	MiniString s = MS_new_string_cap(a, size + 1);
	if (!s.string) return s;

	char *ascii = (char *)arena_alloc(a, size + 1);
	if (!ascii) return s;

	for (size_t i = 0; i < size; ++i) {
		uint8_t c = buf[i];
		ascii[i] = (c >= 32 && c <= 126) ? (char)c : ' ';
	}
	ascii[size] = '\0';

	MS_append_cstr(&s, ascii);
	return s;
}

hv_status_t hv_display_header(Arena *a, FILE *dest, uint32_t w_range,
                              uint32_t w_hex, uint32_t w_ascii) {
	if (!a || !dest) return HV_ERR_INVALID_INPUT;

	uint32_t offset_field = 8 + 2; /* numeric width (8) + minimal trailing spaces */
	if (w_range > offset_field) offset_field = w_range;

	/* compute bytes per row based on hex column width (3 chars per byte) */
	uint32_t bytes_per_row = (w_hex > 0) ? (w_hex / 3) : 1;
	if (bytes_per_row == 0) bytes_per_row = 1;

	/* Print header directly to stream */
	// OFFSET (left-aligned in the offset_field), then hex labels, padding, ASCII label
	if (fprintf(dest, "%-*s", (int)offset_field, "OFFSET") < 0) return HV_ERR_IO_FAIL;

	// hex labels
	for (uint32_t i = 0; i < bytes_per_row; ++i) {
		if (fprintf(dest, "%02x ", i) < 0) return HV_ERR_IO_FAIL;
	}

	// pad any extra hex width
	size_t hex_chars = (size_t)bytes_per_row * 3;
	if (w_hex > hex_chars) {
		size_t pad = (size_t)w_hex - hex_chars;
		for (size_t i = 0; i < pad; ++i) if (fputc(' ', dest) == EOF) return HV_ERR_IO_FAIL;
	}

	// separator
	if (fputs("  ", dest) == EOF) return HV_ERR_IO_FAIL;

	// ascii label left aligned in bytes_per_row width
	if (fprintf(dest, "%-*s", (int)bytes_per_row, "ASCII") < 0) return HV_ERR_IO_FAIL;

	if (fputc('\n', dest) == EOF) return HV_ERR_IO_FAIL;

	return HV_SUCCESS;
}

hv_status_t hv_format_as_table(Arena *a, const uint8_t *buf, size_t buf_size,
                              FILE *dest,
                              uint32_t w_range, uint32_t w_hex, uint32_t bytes_per_row) {
	if (!a || !dest || !buf || buf_size == 0) return HV_ERR_INVALID_INPUT;

	/* reset scratch arena */
	arena_reset(a);

	if (bytes_per_row == 0)
		bytes_per_row = BYTE_COLS_MIN; // fallback to a sane default

	/* offset display field width: 8 hex digits + minimal trailing spaces */
	uint32_t offset_field = 8 + 2;
	if (w_range > offset_field) offset_field = w_range;

	const uint8_t *data = buf;

	/* row buffer used to compose each line before streaming */
	char row[4096];
	size_t rowcap = sizeof(row);

	for (size_t offset = 0; offset < buf_size; offset += bytes_per_row) {
		size_t row_len = buf_size - offset;
		if (row_len > bytes_per_row)
			row_len = bytes_per_row;

		size_t pos = 0;

		/* offset */
		int n = snprintf(row + pos, rowcap - pos, "%08zx", (size_t)offset);
		if (n < 0) return HV_ERR_IO_FAIL;
		if ((size_t)n >= rowcap - pos) return HV_ERR_BUFFER_TOO_SMALL;
		pos += (size_t)n;

		/* pad to offset_field */
		size_t pad_spaces = (offset_field > 8) ? (offset_field - 8) : 0;
		for (size_t p = 0; p < pad_spaces; ++p) {
			if (pos + 1 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
			row[pos++] = ' ';
		}

		/* hex bytes */
		for (size_t i = 0; i < bytes_per_row; ++i) {
			if (i < row_len) {
				int m = snprintf(row + pos, rowcap - pos, "%02x ", (unsigned int)data[offset + i]);
				if (m < 0) return HV_ERR_IO_FAIL;
				if ((size_t)m >= rowcap - pos) return HV_ERR_BUFFER_TOO_SMALL;
				pos += (size_t)m;
			} else {
				if (pos + 3 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
				row[pos++] = ' ';
				row[pos++] = ' ';
				row[pos++] = ' ';
			}
		}

		/* pad to reach w_hex if necessary */
		size_t hex_chars2 = (size_t)bytes_per_row * 3;
		if (w_hex > hex_chars2) {
			size_t extra = (size_t)w_hex - hex_chars2;
			for (size_t q = 0; q < extra; ++q) {
				if (pos + 1 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
				row[pos++] = ' ';
			}
		}

		/* separator */
		if (pos + 2 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
		row[pos++] = ' ';
		row[pos++] = ' ';

		/* ascii */
		for (size_t i = 0; i < row_len; ++i) {
			unsigned char c = data[offset + i];
			char ch = (c >= 32 && c <= 126) ? (char)c : '.';
			if (pos + 1 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
			row[pos++] = ch;
		}

		/* pad ascii */
		if (row_len < bytes_per_row) {
			size_t pad = bytes_per_row - row_len;
			for (size_t k = 0; k < pad; ++k) {
				if (pos + 1 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
				row[pos++] = ' ';
			}
		}

		/* newline */
		if (pos + 1 >= rowcap) return HV_ERR_BUFFER_TOO_SMALL;
		row[pos++] = '\n';
		row[pos] = '\0';

		/* write row to dest */
		if (fputs(row, dest) == EOF) return HV_ERR_IO_FAIL;
	}

	return HV_SUCCESS;
}

hv_status_t hv_format_as_table_color(const uint8_t *buf, FILE *dest, uint32_t cols) {
	// Not implemented yet; placeholder
	if (!buf || !dest || cols == 0) return HV_ERR_INVALID_INPUT;
	(void)buf; (void)dest; (void)cols;
	return HV_SUCCESS;
}
