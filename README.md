# hexview-c

hexview implemented in C.

## Project Structure

```
hexview-c/
	(build/)
	include/
	src/
	tests/
	third_party/
	(libs/)

	README.md
	TODO.txt
	LICENSE
	xmake.lua		-> XMake Build Tool
```

> `libs/` directory is not used, I just wanted to show what a good c-project structure looks like for personal future reference.
> `build/` directory shouldn't be tracked, it contains environment-dependent build artifacts.

Useful Tip: If you're having struggling with the lua language server while writing the `xmake.lua` build script, go to your per-project (or workspace) settings and disable the language server.
XMake provides built-ins and the language server can't access them by default. Maybe also try installing an extension/plugin for your editor (I use Zed btw).

## TODO

- [x] Implement an Arena allocator
- [x] Secure file access (open and read)
- [x] Get the terminal size in columns
- [ ] Implenent a mini string utility
- [x] format the output as range - data - ascii
- [ ] parse arguments meaningfully (color, size, start, end)
- [ ] Make reading and formatting buffered (to be memory efficient)
- [ ] Make sure the hexview-c works cross-platform
- [ ] Implement - cross platform - colored output
