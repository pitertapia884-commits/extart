# EXTART Gecko migration

EXTART is moving from WebKitGTK to Gecko while keeping the application UI in GTK4.

## What Gecko provides

Gecko is Mozilla's web engine. It contains the HTML/CSS stack, DOM, JavaScript through SpiderMonkey, networking, graphics, IPC, security, and the browser process infrastructure used by Firefox.

EXTART will use those engine components without adopting Firefox Desktop's browser UI.

## The important architectural difference

WebKitGTK gives an application a ready-made `WebKitWebView`. Gecko's current source tree does not expose an equivalent Linux GTK4 widget API for external applications. Mozilla's embedding documentation describes the actual embedding path through `BrowsingContext`, `nsDocShell`, and the remote-tab/browser-process infrastructure.

That means EXTART cannot be migrated correctly by replacing `WebKitWebView*` with another widget type. The engine boundary has to be redesigned first.

## Target architecture

```text
GTK4 application (EXTART)
        |
        | native engine adapter
        v
Gecko parent/runtime
        |
        +-- BrowsingContext / navigation
        +-- BrowserHost / BrowserParent
        +-- BrowserChild
        |
        +-- Gecko content process(es)
              |
              +-- nsDocShell
              +-- DOM / layout
              +-- WebRender / graphics
              +-- SpiderMonkey
              +-- Necko networking
```

## Migration stages

### Stage 1 — engine boundary in EXTART — in progress

Completed in this stage:

- Keep the GTK4 browser UI.
- Introduce the engine-neutral `BrowserEngine` interface.
- Move the existing WebKitGTK implementation into `WebKitEngine`.
- Remove WebKitGTK types and calls from `Tab` itself.
- Keep the old WebKit backend temporarily so the application can remain buildable during the transition.
- Stop adding new WebKit-specific functionality to `Tab`.

The remaining WebKit references in `BrowserWindow` and the download subsystem are intentionally left for later sub-stages. This keeps the first refactor small enough to verify before changing more of the application.

### Stage 2 — Gecko runtime

- Build a Gecko/Firefox runtime from Mozilla's source tree.
- Give EXTART its own application/profile data paths.
- Define the Gecko parent-process lifecycle.
- Establish the IPC/process lifecycle needed by a browser tab.

### Stage 3 — native content embedding

- Connect an EXTART tab to a Gecko `BrowsingContext`.
- Connect the tab's native GTK surface to Gecko's widget/rendering path.
- Implement navigation callbacks and loading state.
- Replace WebKit-specific history/download/find callbacks with engine-neutral events.

### Stage 4 — remove WebKitGTK

- Delete `webkit_memory.*` and the WebKit-specific profile/session layer.
- Remove `webkitgtk-6.0` from CMake.
- Remove all `webkit_*` calls from the GTK4 UI.
- Make Gecko the only web engine.

### Stage 5 — strip Firefox-specific application code

Keep the Gecko platform pieces required to browse the modern web. Do not copy Firefox Desktop's UI/services into EXTART unless a component is required by the engine itself.

Potentially removable application-layer pieces include Firefox-specific home/start UI, account/sync integration, Pocket integration, Firefox-specific panels, and other desktop frontend features. The decision must be made per component because some `toolkit` and platform code is shared with Gecko itself.

## Development rule

Do not call EXTART "Gecko-powered" merely because it launches the Firefox executable. Launching Firefox, driving it through WebDriver, or embedding a Firefox window is not the native engine integration we want.

The migration is complete only when EXTART's own tab/browser surface is backed by Gecko.
