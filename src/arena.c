#include "arena.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Region *new_region(size_t capacity) {
	Region *r =
		(Region *)malloc(sizeof(Region) + (sizeof(uintptr_t) * capacity));

	r->next = NULL;
	r->capacity = capacity;
	r->offset = 0;

	return r;
}

void free_region(Region *r) { free(r); }

void *arena_alloc(Arena *a, size_t size_bytes) {
	// calculate how many slots we need using integer rounding
	// Ex:
	// - if user asks for 1 byte -> alloc 1 slot (1 + 7)/8 = 1
	// - if user asks for 9 bytes -> alloc 2 slots (9 + 7)/8 = 2
	size_t size = (size_bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

	// if a->end does not exists, create
	if (a->end == NULL) {
		// if we don't have an end we have not created any regions
		assert(a->begin == NULL);

		size_t capacity = DEFAULT_REGION_SIZE;
		if (capacity < size)
			capacity = size;
		a->end = new_region(capacity);
		a->begin = a->end;
	}

	// if we already have a->end
	// loop through already created regions to find a suitable
	// one to put our data in
	while (a->end->offset + size > a->end->capacity && a->end->next != NULL) {
		a->end = a->end->next;
	}

	// if we cycled through all the pre-allocated regions
	// and none of them was big enough to store our data
	// create a new region that is suitable
	if (a->end->offset + size > a->end->capacity) {
		assert(a->end->next == NULL);

		size_t capacity = DEFAULT_REGION_SIZE;
		if (capacity < size)
			capacity = size;
		a->end->next = new_region(capacity);
		a->end = a->end->next;
	}

	// get the pointer to the current offset
	void *res = &a->end->data[a->end->offset];
	a->end->offset += size;
	return res;
}

void arena_reset(Arena *a) {
	if (a == NULL || a->begin == NULL)
		return;

	Region *curr = a->begin;
	while (curr != NULL) {
		curr->offset = 0;
		curr = curr->next;
	}

	a->end = a->begin;
}

void arena_free(Arena *a) {
	Region *r = a->begin;

	while (r != NULL) {
		Region *r_initial = r;
		r = r->next;
		free_region(r_initial);
	}

	a->begin = NULL;
	a->end = NULL;
}

ArenaMark arena_snapshot(Arena *a) {
	ArenaMark m;

	// uninitialized arena
	if (a->end == NULL) {
		// if end is null, beginning should also be null
		assert(a->begin == NULL);

		m.region = a->begin;
		m.marked_offset = 0;
	} else {
		m.region = a->end;
		m.marked_offset = a->end->offset;
	}

	return m;
}

void arena_rewind(Arena *a, ArenaMark m) {
	if (m.region == NULL) {
		arena_reset(a);
		return;
	}

	m.region->offset = m.marked_offset;

	Region *curr = m.region->next;

	while (curr != NULL) {
		curr->offset = 0;
		curr = curr->next;
	}

	a->end = m.region;
}
