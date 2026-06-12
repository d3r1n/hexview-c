#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "arena.h"

// Assuming your MiniString header is named "ministring.h"
#include "mini_string.h"

// --- Helper Functions ---

void verify_ms(const MiniString *s, const char *expected, size_t expected_cap) {
    assert(s->string != NULL);
    assert(s->length == strlen(expected));
    assert(s->capacity >= expected_cap);
    assert(strncmp(s->string, expected, s->length) == 0);
}

// --- Test Cases ---

void test_MS_new_string(Arena *arena) {
    printf("  -> MS_new_string\n");

    const char *source = "Hello, Arena!";
    MiniString s = MS_new_string(arena, source);

    verify_ms(&s, source, strlen(source));
}

void test_MS_new_string_cap(Arena *arena) {
    printf("  -> MS_new_string_cap\n");

    size_t cap = 50;
    MiniString s = MS_new_string_cap(arena, cap);

    assert(s.string != NULL);
    assert(s.length == 0);
    assert(s.capacity == cap);
}

void test_MS_put_string(Arena *arena) {
    printf("  -> MS_put_string\n");

    MiniString s = MS_new_string_cap(arena, 20);

    MS_Result res = MS_put_string(&s, "Test");
    assert(res == MS_SUCCESS);
    verify_ms(&s, "Test", 20);

    res = MS_put_string(&s, "New");
    assert(res == MS_SUCCESS);
    verify_ms(&s, "New", 20);

    assert(MS_put_string(NULL, "Fail") == MS_ERR_NULL_ARGUMENT);
    assert(MS_put_string(&s, NULL) == MS_ERR_NULL_ARGUMENT);
}

void test_MS_append_cstr(Arena *arena) {
    printf("  -> MS_append_cstr\n");

    MiniString s = MS_new_string_cap(arena, 30);
    assert(MS_put_string(&s, "Hello") == MS_SUCCESS);

    MS_Result res = MS_append_cstr(&s, " World");
    assert(res == MS_SUCCESS);
    verify_ms(&s, "Hello World", 30);

    // Check defense mechanics if string capacity overflows
    MiniString tight = MS_new_string_cap(arena, 5);
    assert(MS_put_string(&tight, "Four") == MS_SUCCESS);
    assert(MS_append_cstr(&tight, "Plus") == MS_ERR_STR_CAP_EXCEEDED);
}

void test_MS_append_char(Arena *arena) {
    printf("  -> MS_append_char\n");

    MiniString s = MS_new_string_cap(arena, 10);
    assert(MS_put_string(&s, "Cat") == MS_SUCCESS);

    MS_Result res = MS_append_char(&s, 's');
    assert(res == MS_SUCCESS);
    verify_ms(&s, "Cats", 10);

    // Verify capacity bounding checks
    MiniString exact = MS_new_string_cap(arena, 3);
    assert(MS_put_string(&exact, "Yes") == MS_SUCCESS);
    assert(MS_append_char(&exact, '!') == MS_ERR_STR_CAP_EXCEEDED);
}

void test_MS_concat_string(Arena *arena) {
    printf("  -> MS_concat_string\n");

    MiniString s1 = MS_new_string(arena, "Foo");
    MiniString s2 = MS_new_string(arena, "Bar");

    MiniString result = MS_concat_string(arena, &s1, &s2);
    verify_ms(&result, "FooBar", 6);
}

void test_MS_new_string_format(Arena *arena) {
    printf("  -> MS_new_string_format\n");

    MiniString s = MS_new_string_format(arena, "Score: %d / %s", 100, "Pass");
    verify_ms(&s, "Score: 100 / Pass", 17);
}

void test_MS_string_slice(Arena *arena) {
    printf("  -> MS_string_slice\n");

    MiniString s = MS_new_string(arena, "Unforgettable");
    MiniString slice;

    MS_Result res = MS_string_slice(&slice, &s, 2, 8);
    assert(res == MS_SUCCESS);
    verify_ms(&slice, "forget", 6);

    assert(MS_string_slice(&slice, &s, 5, 20) == MS_ERR_INVALID_SLICE_RANGE);
    assert(MS_string_slice(&slice, &s, 7, 2) == MS_ERR_INVALID_SLICE_RANGE);
}

void test_MS_to_cstr(Arena *arena) {
    printf("  -> MS_to_cstr\n");

    MiniString s = MS_new_string(arena, "NullTerminated");

    char *cstr = MS_to_cstr(arena, s);
    assert(cstr != NULL);
    assert(strcmp(cstr, "NullTerminated") == 0);
    assert(cstr[14] == '\0');
}

// --- Test Harness Framework ---

typedef void (*TestFunc)(Arena *);

void run_test(Arena *arena, TestFunc test) {
    ArenaMark mark = arena_snapshot(arena);
    test(arena);
    arena_rewind(arena, mark);
}

int main(void) {
    printf("=======================================\n");
    printf("   STARTING MINISTRING LIBRARY TESTS   \n");
    printf("=======================================\n");

    Arena arena = {0};

    run_test(&arena, test_MS_new_string);
    run_test(&arena, test_MS_new_string_cap);
    run_test(&arena, test_MS_put_string);
    run_test(&arena, test_MS_append_cstr);
    run_test(&arena, test_MS_append_char);
    run_test(&arena, test_MS_concat_string);
    run_test(&arena, test_MS_new_string_format);
    run_test(&arena, test_MS_string_slice);
    run_test(&arena, test_MS_to_cstr);

    arena_free(&arena);

    printf("=======================================\n");
    printf("   ALL TESTS PASSED SUCCESSFULLY!  \n");
    printf("=======================================\n");

    return 0;
}
