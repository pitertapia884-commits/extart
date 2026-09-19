# Native Gecko embedding plan

This document is the implementation checklist for replacing the temporary WebKit backend with Gecko.

## 1. Build the Gecko source tree

Use `tools/bootstrap-gecko.sh` to prepare a local Mozilla source tree.

EXTART must treat the Gecko build as a separate build dependency. The Gecko source tree is not copied into the EXTART repository.

## 2. Use Gecko's embedding contracts

Mozilla's source tree contains the classic embedding contracts under `toolkit/components/browser`, including:

- `nsIWebBrowser`
- `nsIWebBrowserChrome`
- `nsIWebNavigation`
- `nsIWebBrowserFind`
- `nsIWindowCreator`
- `nsIWindowWatcher`
- `nsIWindowlessBrowser`

These are the starting interfaces for an embedder. EXTART must not depend on Firefox's `browser/` desktop frontend.

## 3. Create the native host

The future `GeckoEngine` implementation should own a native Gecko browser object and its lifetime.

Conceptually:

```text
GeckoEngine
  |
  +-- Gecko runtime initialization
  +-- XPCOM services
  +-- nsIWebBrowser
  +-- nsIWebNavigation
  +-- nsIWebProgress
  +-- nsIWebBrowserFind
  +-- native widget/content surface
  +-- Gecko shutdown
```

The exact C++ APIs must be taken from the Gecko revision being built. Do not invent headers or method names from older Gecko examples.

## 4. Connect the GTK surface

EXTART's GTK4 UI owns the window and tab layout.

The Gecko content surface must be hosted inside the tab content area. This is the point where the current `BrowserEngine::widget()` abstraction will need to evolve: a future Gecko implementation may not be a normal GTK widget.

Do not solve this by opening Firefox in another window.

## 5. Navigation and events

Map Gecko events into EXTART's engine-neutral callbacks:

- URI/location changes -> `uri_changed`
- document title changes -> `title_changed`
- completed loads -> `load_finished`
- loading failures -> engine error callback
- new-window requests -> EXTART window/tab creation
- downloads -> `DownloadBackend`

Navigation methods must use Gecko's web-navigation interfaces rather than Firefox's desktop tabbrowser frontend.

## 6. Process model

Gecko's current embedding documentation distinguishes in-process and remote-tab embedding. A remote tab uses the parent-side browser host/parent objects to communicate with the content-side browser child and its `nsDocShell`.

EXTART should use the current Gecko process model required by the revision being integrated rather than copying obsolete single-process embedding examples.

## 7. Security

Never load untrusted web content in the Gecko parent process.

Keep web-content operations in the content process and use Gecko's own security/principal infrastructure. EXTART must not replace Gecko's security model with application-side URL checks.

## 8. Remove WebKit

Only after the Gecko implementation can:

- create a tab
- display a normal HTTPS page
- navigate
- go back/forward
- reload
- report title and URI
- search page text
- download a file
- persist the profile
- close cleanly

then:

1. remove `WebKitEngine`
2. remove `webkit_download_backend.cpp`
3. remove WebKitGTK from CMake
4. replace the WebKit profile/session code
5. remove WebKit-specific memory code
6. remove the temporary compatibility layer

## Completion rule

EXTART is not considered Gecko-powered until the content displayed inside EXTART's own tab surface is rendered by Gecko.

Finding or launching a Firefox executable is not sufficient.
