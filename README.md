# EXTART 🍞

<p align="center">
  <img src="imagen.png" alt="EXTART" width="900">
</p>

A minimal web browser for Linux focused on simplicity, efficiency, stability, and low resource usage.

## Engine migration: Gecko

EXTART is being migrated from **WebKitGTK 6.0 to native Gecko embedding**, while keeping the browser's **GTK4 native UI**.

The goal is to use Gecko's web platform — HTML/CSS, JavaScript, networking, DOM, graphics, security, and multi-process architecture — without carrying Firefox's desktop browser UI and services into EXTART.

The migration is being done in small, buildable stages. The current code intentionally keeps WebKitGTK as a temporary backend while the native Gecko host is prepared.

### Current migration state

- GTK4 remains the application UI.
- `BrowserEngine` isolates browser-engine operations from the tab/window UI.
- WebKit-specific navigation and download code has been moved behind engine/backend boundaries.
- `GeckoBackend` now represents a **Gecko runtime directory**, not a Firefox executable.
- EXTART does **not** launch Firefox as a substitute for embedding Gecko.
- Native Gecko embedding is the next engine implementation stage.

## Architecture target

```text
EXTART GTK4 UI
    |
    +-- BrowserWindow
    |     +-- tabs
    |     +-- navigation
    |     +-- history
    |     +-- bookmarks
    |     +-- downloads
    |     +-- settings
    |
    +-- BrowserEngine
          |
          +-- GeckoEngine
                |
                +-- Gecko runtime
                +-- nsIWebBrowser / embedding contracts
                +-- BrowsingContext / nsDocShell
                +-- BrowserHost / BrowserParent / BrowserChild
                +-- Gecko content process(es)
                +-- SpiderMonkey
                +-- Necko
                +-- layout / graphics / WebRender
                +-- security / sandbox
```

## Existing browser features

- Multiple tabs and windows
- Back / Forward / Reload
- Address and search bar
- Persistent browser profile
- Cookies and web data
- Basic configuration storage
- Linux XDG directory support
- GTK4 interface
- Embedded resources using GResource
- History tracking
- Download management
- Bookmarks
- Page search (Ctrl+F)
- Keyboard shortcuts

## Build

### Current temporary build

The repository currently builds against WebKitGTK while the Gecko embedding layer is developed.

Requirements:

- Linux
- C++17 compiler
- CMake
- GTK4
- WebKitGTK 6.0

```bash
git clone https://github.com/pitertapia884-commits/extart.git
cd extart

cmake -S . -B build
cmake --build build
```

### Gecko development build

Mozilla's Firefox source tree contains the Gecko engine and the native embedding contracts used by embedders. EXTART's Gecko integration is intended to use those engine components directly rather than starting Firefox as a separate application.

The repository includes a bootstrap helper for preparing a Gecko source/build tree:

```bash
./tools/bootstrap-gecko.sh
```

The script only prepares the source/build environment. It does not modify EXTART's runtime backend or claim that Gecko embedding is complete.

## Project philosophy

> Use only what is necessary for browsing, and remove everything that does not directly contribute to it.

The main priorities are:

- **Simplicity** - Minimal UI, straightforward navigation
- **Efficiency** - Low RAM and CPU usage
- **Stability** - Reliable and predictable behavior
- **Speed** - Fast startup and page loading

EXTART is not intended to compete with large browsers by adding more features. Every feature must justify its existence.

## Configuration

Settings are stored in `~/.config/extart/settings.ini`.

## Data storage

EXTART respects XDG directory standards:

- **Config:** `~/.config/extart/`
- **Data:** `~/.local/share/extart/`
- **Cache:** `~/.cache/extart/`

## Gecko integration

The Gecko runtime is configured as a directory containing the Gecko runtime resources required by the native embedding layer.

The environment variable is only a configuration input for the future native backend:

```bash
EXTART_GECKO_RUNTIME=/path/to/gecko-runtime ./build/extart
```

It is deliberately **not** interpreted as a Firefox executable path.

## License

License not decided yet.

---

The logo is a toaster 🍞 — simple, reliable, does one thing well.
