#include "arena.h"

#define DEFAULT_ALIGNMENT 8

// helper function to allign a pointer to align memory into multiples of 2
// ex: if align is 8 bytes, we're going to align memory into 8 byte boxes
static uintptr_t align_forward(uintptr_t ptr, size_t align) {
	// Modulo implementation (expensive)
	// size_t remainder = ptr % align; // how many bytes we're past the boundary
	// // already aligned
	// if (remainder == 0)
	// 	return ptr;
	// // (align - remainder) is how many bytes we need to shift
	// // so we can be at the beginning of the next boundary
	// return ptr + (align - remainder);

	// bitwise alignment (cheap)
	return (ptr + (align - 1)) & ~(align - 1);
}

Arena *arena_init(size_t capacity) {
	Arena *a = (Arena *)malloc(sizeof(Arena));

	// check if arena is allocated
	if (!a)
		return NULL;

	// init arena
	a->offset = 0;
	a->size = capacity;
	a->buffer = (uint8_t *)malloc(capacity);

	// check if buffer is allocated
	if (!a->buffer) {
		free(a);
		return NULL;
	}

	return a;
}

/*
 * @brief Allocates memory in the arena
 * @param a Arena pointer
 * @param size size of the memory to be allocated
 * @return NULL if allocation was not successful
 * @return void* pointer to the start of the allocated memory
 */
void *arena_alloc(Arena *a, size_t size) {
	// beginning of the buffer + current offset we're at
	uintptr_t current_ptr = (uintptr_t)a->buffer + (uintptr_t)a->offset;
	uintptr_t aligned_ptr = align_forward(current_ptr, DEFAULT_ALIGNMENT);

	// calculate how much we're going to move after aligning and
	// allocating the requested amount
	size_t new_offset = (aligned_ptr - (uintptr_t)a->buffer) + size;

	// check if we have enough space in the arena
	if (new_offset > a->size)
		return NULL;

	void *ptr = (void *)aligned_ptr;
	a->offset = new_offset;

	return ptr;
}

void arena_reset(Arena *a) { a->offset = 0; }

void arena_free(Arena *a) {
	free(a->buffer); // free the buffer
	free(a);		 // free the arena
}
