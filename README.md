# hexview-c

hexview implemented in C.

## Project Structure

```
hexview-c/
	(build/)
	include/
	src/
	test/
	third_party/
	(libs/)

	README.md
	TODO.txt
	LICENSE
	xmake.lua		-> XMake Build Tool
```

> `libs/` directory is not used, I just wanted to show what a good c-project structure looks like for personal future reference.


> `build/` directory shouldn't be tracked, it contains environment-dependent build artifacts.


> `third_party/` is where you should put any third party dependencies, but this project doesn't have any yet. (probably won't have any, it's a simple project)

Useful Tip: If you're struggling with the lua language server while writing the `xmake.lua` build script, go to your per-project (or workspace) settings and disable the language server.
XMake provides built-ins and the language server can't access them by default. Maybe also try installing an extension/plugin for your editor (I use Zed btw).

## Usage

Basic usage:

```
./build/hexview <file_path> [options]
```

Options:

- `-n`, `--no-header`    Do not print the header
- `-c`, `--color`        Force colored output (default: no color)
- `--no-color`           Explicitly disable color output (takes precedence)
- `-h`, `--help`         Show this help message

Notes on color control:

- Environment variables:
  - `HEXVIEW_COLOR`: set to a truthy value (`1`, `true`, `yes`, `on`, `y`) to enable color by default
  - `NO_COLOR`: when present, disables color output (this now takes precedence over CLI flags)
- CLI flags override `HEXVIEW_COLOR` but not `NO_COLOR` (see precedence above).

The header is printed to the output stream and the program draws a separator line under the header to visually separate it from the data rows.

## Build & test

Build with XMake (project includes `xmake.lua`):

```sh
xmake -v
```

Run the unit tests and example:

```sh
xmake test

./build/hexview test.txt
```

## TODO

- [x] Implement an Arena allocator
- [x] Secure file access (open and read)
- [x] Get the terminal size in columns
- [x] format the output as range - data - ascii
- [x] Make reading and formatting stream-based (don't read the whole file into memory)
- [x] Implement a mini string utility
- [ ] parse arguments meaningfully (size, start, end)
- [x] Implement - cross platform - colored output
- [ ] Make sure the hexview-c works on all target platforms
