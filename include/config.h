#pragma once

#define MAX_COLUMNS 128
#define MIN_COLUMNS 40

#define BYTE_COLS_MIN 8
#define BYTE_COLS_MAX 16
#define HEX_COLS_MIN 40
#define HEX_COLS_MAX 80
#define ASCII_COLS_MIN 20
#define ASCII_COLS_MAX 40

#define FILE_READ_CHUNK_SIZE (1024 * 2)

/* Formatting / layout magic numbers */
#define OFFSET_NUMERIC_WIDTH 8
#define OFFSET_TRAILING_SPACES 2
#define OFFSET_FIELD_MIN (OFFSET_NUMERIC_WIDTH + OFFSET_TRAILING_SPACES)

#define HEX_CHARS_PER_BYTE 3

/* Row buffer sizes (non-colored and colored) */
#define ROW_BUFFER_SIZE 4096
#define COLOR_ROW_BUFFER_SIZE 8192

/* ANSI color sequences (used when color output is enabled). Kept in config
   so they are easily adjustable. */
#define ANSI_SEQ_RESET "\x1b[0m"
#define ANSI_SEQ_HEX "\x1b[33m"             /* yellow */
#define ANSI_SEQ_ASCII_PRINTABLE "\x1b[32m" /* green */
#define ANSI_SEQ_ASCII_NONPRINT "\x1b[90m"  /* bright black / gray */
