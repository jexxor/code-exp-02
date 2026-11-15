# macOS port status (15 November 2026)

The experimental branch `macos-support` builds a native `.app` for macOS 14+ on Apple Silicon and Intel. CI compiles the complete application, checks the bundle's plist, fonts, and agent scripts, and exercises keyboard, mouse, scroll, and clipboard translation with a headless SDL window. The interactive UI has not yet been run on a Mac with a display.

| Area | Current state |
| --- | --- |
| Window and drawing | SDL3 requests a macOS window with an OpenGL 4.1 context; the renderer keeps the same drawing code as Linux. An adapter maps SDL3 window, mouse, keyboard, text, and clipboard events to the canvas handlers. |
| Project and language services | FSEvents watches file changes. Pipe wakeups, subprocesses, `clangd`, `gopls`, and the agent socket compile on both Mac architectures. |
| Desktop integration | The folder picker uses `osascript`, links use `open`, and settings live under `~/Library/Application Support/source-canvas/`. |
| Bundle | CMake produces `white_window.app` with fonts, `agent_bridge.py`, `intents.py`, and a High DPI plist. Homebrew dynamic libraries are not bundled. |

The remaining acceptance check needs a Mac with a graphical session: open a project, type and paste in search and chat, pan and zoom on a Retina display, follow Go/C++ definitions, watch file changes, and verify agent interaction. The system theme watcher still uses Linux desktop settings; the explicit Monokai setting works independently. Signing, notarization, and portable library bundling belong to the later release stage.

Build commands and dependencies are in the [README](../README.md#experimental-macos-build). The [macOS CI workflow](../.github/workflows/macos-port.yml) records the two architecture builds and focused platform checks.
