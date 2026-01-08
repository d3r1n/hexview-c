#pragma once

// Arena allocator and related utilities for hexview
// NOTE: I love arenas

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	size_t size;
	size_t offset;
	uint8_t *buffer; // Buffer chunk size: 1 byte (8 bits)
} Arena;

Arena *arena_init(size_t capacity);
void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_free(Arena *a);
