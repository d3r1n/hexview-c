#pragma once

// Arena allocator and related utilities for hexview
// NOTE: I love arenas

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_REGION_SIZE (8 * 1024) // 8KB

typedef struct Region Region;

struct Region {
	Region *next;
	size_t capacity;
	size_t offset;
	uintptr_t data[];
};

typedef struct {
	Region *begin, *end;
} Arena;

typedef struct {
	Region *region;
	size_t marked_offset;
} ArenaMark;

Region *new_region(size_t capacity);
void free_region(Region *r);

void *arena_alloc(Arena *a, size_t size);
void arena_reset(Arena *a);
void arena_free(Arena *a);

ArenaMark arena_snapshot(Arena *a);
void arena_rewind(Arena *a, ArenaMark m);
