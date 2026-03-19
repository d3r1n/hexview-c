#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "utils.h"

int main(void) {
	char *path = "./test.txt";

	FILE *f;
	size_t f_size;

	hv_open_file(path, &f);
	hv_file_size(&f_size, f);

	Arena *a = arena_init(1024 * 8);
	uint8_t *byte_buf = (uint8_t *)arena_alloc(a, 1024 * 2);
	char *out_buf = (char *)arena_alloc(a, 1024 * 5);

	uint32_t cur_byte_range = 0;
	bool include_header = true;

	while (!feof(f) && cur_byte_range < f_size) {
		// read
		size_t amount_read;
		hv_file_read(byte_buf, 1024 * 2, f, &amount_read);

		hv_format_as_table(byte_buf, amount_read, out_buf, 1024 * 5,
						   hv_get_terminal_cols(), include_header,
						   &cur_byte_range);

		include_header = false;

		printf("%s", out_buf);
	}

	printf("\n");

	return 0;
}
