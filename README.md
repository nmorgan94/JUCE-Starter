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


## Adding Assets

1. Place your asset files in the `Assets/` directory
2. Reconfigure and rebuild the project:
   ```bash
   cmake --workflow --preset debug
   ```
3. Access assets in your code:
   ```cpp
   #include <BinaryData.h>
   
   // Example: Load an image
   auto image = juce::ImageCache::getFromMemory(
       BinaryData::your_image_png,
       BinaryData::your_image_pngSize
   );
   ```

**How It Works:**
- Assets are automatically converted to C++ code during build
- The `BinaryData.h` header is generated in the build directory
- Asset names are derived from filenames
- All assets are available in the `BinaryData` namespace

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
   (configure with `-DENABLE_PLUGINVAL=OFF -DENABLE_UNIT_TESTS=OFF` to fetch only JUCE)
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
`JuceStarter_Tests` executable that links the plugin's code and drives the processor directly,
checking parameters, audio output, bus layouts and state save/restore. Run them with the `unit`,
`unit-release`, `test` or `test-release` workflow (see [Commands](#commands)).

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
and runs the unit tests (not pluginval). It only runs when started by hand: **Actions → CI → Run
workflow** on GitHub, or `gh workflow run ci.yml`.

Each run's page lists the plugins under **Artifacts** for 30 days: `JuceStarter-macOS.tar.gz` (VST3
and AU) and `JuceStarter-Windows.tar.gz` (VST3). They are unsigned.

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
project(JuceStarter)  # Change project name

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME MyCompanyName            # Your company name
    PLUGIN_MANUFACTURER_CODE Mycn         # 4-char manufacturer ID
    PLUGIN_CODE Plgn                      # 4-char unique plugin ID
    VERSION 0.0.0                         # Version number
    FORMATS VST3 AU Standalone            # Plugin formats to build
    # ... other options
)
```
