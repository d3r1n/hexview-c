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

## TODO

- [x] Implement an Arena allocator
- [x] Secure file access (open and read)
- [x] Get the terminal size in columns
- [x] format the output as range - data - ascii
- [x] Make reading and formatting stream-based (don't read the whole file into memory)
- [x] Implenent a mini string utility
- [ ] parse arguments meaningfully (color, size, start, end)
- [ ] Make sure the hexview-c works cross-platform
- [ ] Implement - cross platform - colored output
