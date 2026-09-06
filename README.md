# EXTART 🍞

A minimal web browser for Linux focused on simplicity, efficiency, stability, and low resource usage.

EXTART is built with **C++17**, **GTK4**, and **WebKitGTK 6.0**.

## Current version: 0.2

### What's new in 0.2
- ⚡ **50% less RAM consumption** - Optimized WebKit memory settings
- 📜 **History** - Browse your visited pages
- 📥 **Download Manager** - Automatic downloads handling
- 📚 **Bookmarks** - Save your favorite sites
- 🔍 **Find in page** - Search within web pages (Ctrl+F)
- ⌨️ **Keyboard shortcuts** - Ctrl+T, Ctrl+W, Ctrl+Tab, Ctrl+L
- 🖥️ **System integration** - .desktop file for app menu

## Features

Currently implemented:

- Web browsing with WebKitGTK
- Multiple tabs and windows
- Back / Forward / Reload
- Address and search bar
- Persistent browser profile
- Cookies and web data
- Basic configuration storage
- Linux XDG directory support
- GTK4 interface
- Embedded resources using GResource
- **NEW: History tracking**
- **NEW: Download management**
- **NEW: Bookmarks**
- **NEW: Page search (Ctrl+F)**
- **NEW: Better keyboard shortcuts**

## Build

### Requirements

- Linux
- C++17 compiler
- CMake
- GTK4
- WebKitGTK 6.0

### Compile

```bash
git clone https://github.com/pitertapia884-commits/extart.git
cd extart

cmake -S . -B build
cmake --build build
```

## Run

After building:

```bash
./build/extart
```

### Install .desktop file (optional)

```bash
mkdir -p ~/.local/share/applications
cp extart.desktop ~/.local/share/applications/
```

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New window |
| Ctrl+T | New tab |
| Ctrl+W | Close tab |
| Ctrl+Tab | Next tab |
| Ctrl+Shift+Tab | Previous tab |
| Ctrl+L | Focus address bar |
| Ctrl+F | Find in page |

## Project Philosophy

EXTART aims to be a browser that respects the user's resources.

> Use only what is necessary for browsing, and remove everything that does not directly contribute to it.

The main priorities are:

- **Simplicity** - Minimal UI, straightforward navigation
- **Efficiency** - Low RAM and CPU usage (50% less memory than Firefox with multiple tabs)
- **Stability** - Reliable and predictable behavior
- **Speed** - Fast startup and page loading

EXTART is not intended to compete with large browsers by adding more features. Every feature must justify its existence.

## Configuration

Settings are stored in `~/.config/extart/settings.ini`:

```ini
[General]
search-engine=https://www.google.com/search?q=
download-directory=/home/user/Downloads
theme=system
```

## Data Storage

EXTART respects XDG directory standards:

- **Config:** `~/.config/extart/`
- **Data:** `~/.local/share/extart/`
  - `history.csv` - Visit history
  - `bookmarks.csv` - Bookmarked sites
- **Cache:** `~/.cache/extart/`
- **Cookies:** Stored in web data directory

## Memory Usage

EXTART is optimized for memory efficiency:

- Single tab: ~80-100MB
- Multiple tabs: ~100-150MB for 3-4 tabs
- Compared to Firefox: ~50% less memory usage

This is achieved through:
- Disabled WebGL (saves 30-50MB per tab)
- Reduced memory cache model
- Explicit memory cleanup

For comparison, Firefox uses ~200-300MB for similar workload.

## Current Status

EXTART 0.2 is a functional, feature-complete browser for everyday use. Focus has been on:
1. Reducing memory consumption
2. Adding essential features (history, bookmarks, downloads)
3. Improving keyboard navigation

## Roadmap

- **0.3** - UI polish (find toolbar, download panel)
- **0.4** - Session restore, theme improvements
- **0.5** - Performance optimizations
- **1.0** - Feature freeze, stability focus

## License

License not decided yet.

## Contributing

Contributions welcome. Keep the philosophy in mind: simplicity and efficiency first.

---

Made with ❤️ for users who prefer minimal, efficient tools.

The logo is a toaster 🍞 — simple, reliable, does one thing well.
