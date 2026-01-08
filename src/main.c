#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "utils.h"

#define SIZE_4KB (1024 * 4)

int main(void) {
	// 4kb arena

	const char *filename = "/Users/d3r1n/Desktop/projects/hexview-c/xmake.lua";

	size_t file_size;
	uint8_t *contents;
	FILE *file;

	hv_open_file(filename, &file);
	hv_file_size(&file_size, file);

	Arena *a = arena_init(SIZE_4KB + file_size);

	hv_status_t result =
		hv_file_read(a, &contents, file_size / sizeof(*contents), file);

	if (result != HEXVIEW_NO_ERROR) {
		printf("%d", result);
		return 1;
	}

	hv_string *dest;
	hv_format_as_table(a, contents, file_size, &dest, hv_get_terminal_cols());

	printf("%.*s\n", (int)dest->length, dest->str);
	arena_free(a);
	return 0;
}
