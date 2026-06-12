#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "config.h"
#include "mini_string.h"
#include "utils.h"

#define CLAMP(x, MIN, MAX) fmax(MIN, fmin(x, MAX))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file_path> [-n | --no-header]\n", argv[0]);
		return 1;
	}

	const char *path = argv[1];

	if (strcmp(path, "--help") == 0 || strcmp(path, "-h") == 0) {
		printf("Usage: %s <file_path>\n", argv[0]);
		printf("Options:\n");
		printf("  --help, -h    Show this help message\n");
		return 0;
	}

	bool no_header = false;
	if (argc > 2 &&
		(strcmp(argv[2], "-n") == 0 || strcmp(argv[2], "--no-header") == 0)) {
		no_header = true;
	}

	FILE *f;
	size_t f_size;

	if (hv_open_file(path, &f) != HV_SUCCESS) {
		fprintf(stderr, "Failed to open file: %s\n", path);
		return 1;
	}

	if (hv_file_size(&f_size, f) != HV_SUCCESS) {
		fprintf(stderr, "Failed to get file size: %s\n", path);
		return 1;
	}

	Arena a = {0};
	Arena temp_arena = {0};
	Arena *context = &a;

	uint32_t terminal_cols = hv_get_terminal_cols();
	uint32_t available_width = terminal_cols - 10;

	// calculated widths per table column
	uint32_t w_range = (uint32_t)(available_width * 0.15f);
	uint32_t w_ascii = (uint32_t)(available_width * 0.25f);
	uint32_t w_hex = (uint32_t)(available_width * 0.60f);

	w_range = CLAMP(w_range, BYTE_COLS_MIN, BYTE_COLS_MAX);
	w_ascii = CLAMP(w_ascii, ASCII_COLS_MIN, ASCII_COLS_MAX);
	w_hex = CLAMP(w_hex, HEX_COLS_MIN, HEX_COLS_MAX);

	uint32_t bytes_per_row =
		w_hex / 3; // XX[space] XX[space] ; 3 chars per byte

	// TODO: if --no-header / -n do not print
	if (!no_header) {
		// stream header to stdout
		if (hv_display_header(context, stdout, w_range, w_hex, w_ascii) !=
			HV_SUCCESS) {
			fprintf(stderr, "Failed to print header\n");
		}
	}

	uint8_t *chunk = (uint8_t *)arena_alloc(context, FILE_READ_CHUNK_SIZE);
	long cur_file_pos = hv_file_get_pos(f);
	if (cur_file_pos < 0)
		cur_file_pos = 0;

	context = &temp_arena;

	while (!feof(f) && (size_t)cur_file_pos < f_size) {
		uint32_t read_size;
		hv_status_t status =
			hv_file_read(chunk, FILE_READ_CHUNK_SIZE, f, &read_size);

		if (status != HV_SUCCESS) {
			printf("Error reading file\n");
			exit(1);
		}

		cur_file_pos += read_size;
		status = hv_format_as_table(context, chunk, read_size, stdout, w_range,
									w_hex, bytes_per_row);

		if (status != HV_SUCCESS) {
			fprintf(stderr, "Failed to format table rows (err=%d)\n",
					(int)status);
			exit(1);
		}
	}
	printf("\n");

	arena_free(&a);
	arena_free(&temp_arena);
	return 0;
}
