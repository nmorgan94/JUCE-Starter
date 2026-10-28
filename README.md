# JuceStarter

A JUCE audio plugin template for creating VST3, AU, and Standalone audio plugins on macOS and Windows.

## Features

- VST3, AU (macOS only), and Standalone formats
- Builds on macOS and Windows with the same preset commands
- Modern C++23
- Automatic JUCE dependency management via CPM
- Binary asset support
- Plugin validation via pluginval, wired up as CTest tests
- Unit tests with Catch2, run through CTest alongside pluginval
- One parameter list, `Source/shared/ParameterSpecs.h`, read by the plugin, the wasm engine and the web UI
- A playable web demo: the DSP built as WebAssembly, with a React copy of the editor
- An optional WebView editor, the same React editor inside the plugin


## Adding Assets

Files in `Assets/` are embedded as `BinaryData` (reconfigure after adding one):

```cpp
#include <BinaryData.h>

auto image = juce::ImageCache::getFromMemory (BinaryData::your_image_png, BinaryData::your_image_pngSize);
```

## Prerequisites

- CMake 3.25 or higher
- C++23 compatible compiler:
  - **macOS**: Xcode or its Command Line Tools (the `xcode` preset needs the full Xcode app)
  - **Windows**: Visual Studio 2022 or later with the "Desktop development with C++" workload
- Git (for downloading JUCE)

## Commands

Every command is the same on macOS and Windows, except the rows marked **macOS** or **Windows**.
Each workflow configures, builds and, for tests, runs in one go.

| Task | Command |
|---|---|
| Build the plugin (Debug) | `cmake --workflow --preset debug` |
| Build the plugin (Release) | `cmake --workflow --preset release` |
| Run the unit tests | `cmake --workflow --preset unit` |
| Run the unit tests against a Release build | `cmake --workflow --preset unit-release` |
| Run the unit tests and pluginval | `cmake --workflow --preset test` |
| Run all tests against a Release build | `cmake --workflow --preset test-release` |
| Build the plugin with the WebView editor (Release) | `cmake --workflow --preset webview` |
| Launch the Standalone app (Debug) | `cmake --build --preset run` |
| Install the plugin for your DAW: **macOS** | Automatic on every build |
| Install the plugin for your DAW: **Windows** | `cmake --build --preset install-windows`, from an administrator prompt |
| Open in an IDE for debugging: **macOS** | `cmake --preset xcode`, then open `build-xcode/JuceStarter.xcodeproj` |
| Open in an IDE for debugging: **Windows** | Open `build-debug\JuceStarter.sln` in Visual Studio after a Debug workflow |

`run` and `install-windows` use the Debug build, so run a Debug workflow such as
`cmake --workflow --preset debug` once first.

The individual steps still work on their own, for example to pass a `-D` option:
```bash
cmake --preset debug -DPLUGINVAL_STRICTNESS=5   # configure
cmake --build --preset test                     # build
ctest --preset debug                            # test
```

`cmake --list-presets=all` lists the presets for your platform: `xcode` only appears on macOS, and
`install-windows` only on Windows.

### What Happens During Build

1. **First time only**: CMake downloads JUCE, pluginval and Catch2 automatically via CPM
   into `.cpm-cache/`, shared by every preset and by `wasm/`
   (configure with `-DENABLE_PLUGINVAL=OFF -DENABLE_UNIT_TESTS=OFF` to fetch only JUCE).
   A build folder made before the cache needs its `_deps/juce-*` removed first
2. **Every build**: Compiles your plugin in the selected configuration
3. **macOS only, by default**: Installs plugins to system directories
   (see [System Installation](#system-installation))

## Debugging in Xcode

Generate and open the project with the `xcode` preset (see [Commands](#commands)). To debug the
plugin inside a host:

1. Select your plugin target from the scheme dropdown
2. Go to **Product → Scheme → Edit Scheme**
3. Click **Run** on the left sidebar
4. Under **Executable**, choose **Other** and navigate to executable.

## Unit Tests

Unit tests live in `Tests/` and use [Catch2](https://github.com/catchorg/Catch2). They build into a
`JuceStarter_Tests` executable that links the plugin's code and drives the processor directly.
Run them with the `unit`, `unit-release`, `test` or `test-release` workflow (see [Commands](#commands)).

Like pluginval, the tests are not part of a plain build: the `debug` workflow builds only the plugin.

Add a `.cpp` file to `Tests/` and reconfigure to pick it up. `Tests/TestHelpers.h` has helpers for
setting parameters and processing buffers. Set `-DENABLE_UNIT_TESTS=OFF` to skip them entirely.

## Validating the Plugin

[pluginval](https://github.com/Tracktion/pluginval) loads the built plugin as a host would and
tests it for stability. It is built from source on demand, so there is nothing to install.

```bash
cmake --build --preset debug --target validate   # builds the plugin and pluginval, then validates
```

To run pluginval together with the unit tests, use the `test` workflow (see [Commands](#commands)).

Logs land in `build-debug/pluginval-logs/`. Strictness defaults to 10; use
`-DPLUGINVAL_STRICTNESS=5` (range 1–10) for a faster run, or `-DENABLE_PLUGINVAL=OFF` to skip
pluginval entirely.

The AU test validates the installed component in `~/Library/Audio/Plug-Ins/Components`, since macOS
resolves Audio Units through its registry rather than by path - so it needs `PLUGIN_COPY_AFTER_BUILD`
left on. Steinberg's VST3 conformance validator is off by default, as it pulls ~300MB of SDK for one
extra test; enable it with `-DPLUGINVAL_VST3_VALIDATOR=ON`.

## CI

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) builds the Release plugin on macOS and Windows
and runs the unit tests. It only runs when started by hand: **Actions → CI → Run workflow** on
GitHub, or `gh workflow run ci.yml`. Tick **Also run pluginval** (or pass `-f pluginval=true`) to
validate the plugin too, which takes longer.

Each run's page lists the plugins under **Artifacts** for 30 days: `JuceStarter-macOS.tar.gz` (VST3
and AU) and `JuceStarter-Windows.tar.gz` (VST3). They are unsigned.

## Web Demo and WebView Editor

Needs Emscripten (`brew install emscripten`) and Node 24+.

| Task | Command |
|---|---|
| Build and test the demo (engine and UI tests) | `cd wasm && cmake --workflow --preset wasm` |
| Serve the demo with live reload at http://localhost:8099 | `cd webui && npm run serve` |
| Typecheck and test the UI on their own | `cd webui && npm run typecheck && npm test` |
| Build the plugin with the WebView editor | `cmake --workflow --preset webview` |

- The demo builds to `wasm/build-wasm/<project>-<version>/`, named from `project()` and `VERSION` in
  `CMakeLists.txt`. Upload that folder; the site embeds its `index.html` in an iframe.
  [`wasm.yml`](.github/workflows/wasm.yml) builds the same folder on GitHub, run by hand.
- The demo's input is made by `webui/src/demo/loop.ts`. For a recording, add it to
  `webui/public/demo/` as AAC (`.m4a`) so Safari can decode it, and decode it in `startAudio`.
- The `webview` build installs over the native one; run the `release` workflow to switch back.
- Delete `wasm/patches/juce-wasm.patch` once JUCE fixes its wasm support.

## Plugin Locations

### Build Artifacts

Paths are shown for macOS. On Windows the Standalone is `JuceStarter.exe` instead of `JuceStarter.app`,
and there is no AU.

**Debug Build:**
- Standalone: `build-debug/JuceStarter_artefacts/Debug/Standalone/JuceStarter.app`
- AU: `build-debug/JuceStarter_artefacts/Debug/AU/JuceStarter.component`
- VST3: `build-debug/JuceStarter_artefacts/Debug/VST3/JuceStarter.vst3`

**Release Build:**
- Standalone: `build-release/JuceStarter_artefacts/Release/Standalone/JuceStarter.app`
- AU: `build-release/JuceStarter_artefacts/Release/AU/JuceStarter.component`
- VST3: `build-release/JuceStarter_artefacts/Release/VST3/JuceStarter.vst3`

### System Installation

**macOS**: copied automatically after every build. Turn this off with
`-DPLUGIN_COPY_AFTER_BUILD=OFF`, though the AU pluginval test then skips.
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/JuceStarter.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/JuceStarter.component`

**Windows**: installed on demand with the `install-windows` preset (see [Commands](#commands)),
because the VST3 folder needs administrator rights.
- **VST3**: `C:\Program Files\Common Files\VST3\JuceStarter.vst3`


## Customization

Edit `CMakeLists.txt` to customize:

```cmake
project(JuceStarter)  # Change project name; also names the web demo folder

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME MyCompanyName            # Your company name
    PLUGIN_MANUFACTURER_CODE Mycn         # 4-char manufacturer ID
    PLUGIN_CODE Plgn                      # 4-char unique plugin ID
    VERSION 0.0.1                         # Version number
    FORMATS VST3 AU Standalone            # Plugin formats to build
    # ... other options
)
```
