#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "config.h"
#include "mini_string.h"
#include "utils.h"

static inline uint32_t clamp_u32(uint32_t x, uint32_t min_v, uint32_t max_v) {
	if (x < min_v)
		return min_v;
	if (x > max_v)
		return max_v;
	return x;
}

static void print_usage(FILE *out, const char *prog) {
	fprintf(out, "Usage: %s <file_path> [options]\n", prog);
	fprintf(out, "Options:\n");
	fprintf(out, "  -n, --no-header    Do not print header\n");
	fprintf(out,
	        "  -c, --color        Force colored output (default: no color)\n");
	fprintf(out, "  --no-color         Explicitly disable color output\n");
	fprintf(out, "  -h, --help         Show this help message\n");
}

int main(int argc, char **argv) {
	if (argc < 2) {
		print_usage(stderr, argv[0]);
		return 1;
	}

	const char *path = argv[1];

	if (strcmp(path, "--help") == 0 || strcmp(path, "-h") == 0) {
		print_usage(stdout, argv[0]);
		return 0;
	}

	bool no_header = false;
	bool use_color = false;

	/* environment-controlled color: priority (low to high):
	   - default (false)
	   - HEXVIEW_COLOR env var (if present)
	   - NO_COLOR env var (if present) disables unless CLI explicitly overrides
	   - CLI flags (-c / --color or --no-color) override env vars
	*/
	char *env_hex_color = getenv("HEXVIEW_COLOR");
	if (env_hex_color) {
		// parse common truthy values
		char buf[16];
		size_t j = 0;
		for (size_t i = 0; env_hex_color[i] && j + 1 < sizeof(buf); ++i)
			buf[j++] = (char)tolower((unsigned char)env_hex_color[i]);
		buf[j] = '\0';
		if (strcmp(buf, "1") == 0 || strcmp(buf, "true") == 0 ||
		    strcmp(buf, "yes") == 0 || strcmp(buf, "on") == 0 ||
		    strcmp(buf, "y") == 0) {
			use_color = true;
		}
	}
	/* NO_COLOR disables colors unless CLI explicitly sets color flags */
	int no_color_env = getenv("NO_COLOR") != NULL;

	// parse optional flags after the path; CLI overrides env vars
	for (int i = 2; i < argc; ++i) {
		if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-header") == 0) {
			no_header = true;
		} else if (strcmp(argv[i], "-c") == 0 ||
		           strcmp(argv[i], "--color") == 0) {
			use_color = true;
		} else if (strcmp(argv[i], "--no-color") == 0) {
			use_color = false;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "--help") == 0) {
			print_usage(stdout, argv[0]);
			return 0;
		} else {
			// unknown flag, ignore or warn
			fprintf(stderr, "Warning: unknown option '%s'\n", argv[i]);
		}
	}

	// apply NO_COLOR: always disable color if NO_COLOR env var is present
	// (it now takes precedence over CLI flags)
	if (no_color_env) {
		use_color = false;
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

	w_range = clamp_u32(w_range, BYTE_COLS_MIN, BYTE_COLS_MAX);
	w_ascii = clamp_u32(w_ascii, ASCII_COLS_MIN, ASCII_COLS_MAX);
	w_hex = clamp_u32(w_hex, HEX_COLS_MIN, HEX_COLS_MAX);

	uint32_t bytes_per_row =
	    w_hex / 3; // XX[space] XX[space] ; 3 chars per byte

	// TODO: if --no-header / -n do not print
	if (!no_header) {
		// stream header to stdout
		if (hv_display_header(context, stdout, w_range, w_hex, w_ascii,
		                      use_color) != HV_SUCCESS) {
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
		hv_status_t status = hv_file_read(chunk, FILE_READ_CHUNK_SIZE, f, &read_size);

		if (status != HV_SUCCESS) {
			printf("Error reading file\n");
			exit(1);
		}

		size_t base_offset = (size_t)cur_file_pos;

		if (use_color) {
			status = hv_format_as_table_color(context, chunk, read_size, stdout, base_offset,
			                                  w_range, w_hex, bytes_per_row);
		} else {
			status = hv_format_as_table(context, chunk, read_size, stdout, base_offset,
			                            w_range, w_hex, bytes_per_row);
		}

		if (status != HV_SUCCESS) {
			fprintf(stderr, "Failed to format table rows (err=%d)\n",
			        (int)status);
			exit(1);
		}

		cur_file_pos += read_size;
	}
	printf("\n");

	arena_free(&a);
	arena_free(&temp_arena);
	return 0;
}
