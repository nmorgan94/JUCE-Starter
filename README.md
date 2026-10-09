# JuceStarter

A JUCE audio plugin template for creating VST3, AU, and Standalone audio plugins.

## Features

- VST3, AU, and Standalone formats
- Modern C++23
- Automatic JUCE dependency management via CPM
- Binary asset support
- Plugin validation via pluginval, wired up as CTest tests
- Unit tests with Catch2, run through CTest alongside pluginval


## Adding Assets

1. Place your asset files in the `Assets/` directory
2. Reconfigure and rebuild the project:
   ```bash
   cmake --preset debug
   cmake --build --preset debug
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
- Git (for downloading JUCE)

## Building

This project uses CMake Presets for easy Debug and Release builds.

### Quick Start (Recommended)

**Debug Build:**
```bash
cmake --preset debug
cmake --build --preset debug
```

**Release Build:**
```bash
cmake --preset release
cmake --build --preset release
```

### Running the Standalone App

**macOS:**
```bash
# Debug version
open build-debug/JuceStarter_artefacts/Debug/Standalone/JuceStarter.app

# Release version
open build-release/JuceStarter_artefacts/Release/Standalone/JuceStarter.app
```

### What Happens During Build

1. **First time only**: CMake downloads JUCE, pluginval and Catch2 automatically via CPM
   (configure with `-DENABLE_PLUGINVAL=OFF -DENABLE_UNIT_TESTS=OFF` to fetch only JUCE)
2. **Every build**: Compiles your plugin in the selected configuration
3. **If `COPY_PLUGIN_AFTER_BUILD` is enabled**: Installs plugins to system directories

## Debugging in Xcode

To debug the plugin in Xcode with an executable:

### 1. Generate Xcode Project

```bash
cmake -B build-xcode -G Xcode
open build-xcode/JuceStarter.xcodeproj
```

### 2. Configure Debugging

1. Select your plugin target from the scheme dropdown
2. Go to **Product → Scheme → Edit Scheme**
3. Click **Run** on the left sidebar
4. Under **Executable**, choose **Other** and navigate to executable.

## Unit Tests

Unit tests live in `Tests/` and use [Catch2](https://github.com/catchorg/Catch2). They build into a
`JuceStarter_Tests` executable that links the plugin's code and drives the processor directly,
checking parameters, audio output, bus layouts and state save/restore.

```bash
cmake --build --preset unit && ctest --preset unit    # builds and runs the unit tests
cmake --build --preset test && ctest --preset debug   # same, plus pluginval
cmake --build --preset test-release && ctest --preset release   # all tests against a Release build
```

Like pluginval, the tests are not part of a plain build (`cmake --build --preset debug` builds only
the plugin), so build with the `unit` or `test` preset before running `ctest`.

Add a `.cpp` file to `Tests/` and reconfigure to pick it up. `Tests/TestHelpers.h` has helpers for
setting parameters and processing buffers. Set `-DENABLE_UNIT_TESTS=OFF` to skip them entirely.

## Validating the Plugin

[pluginval](https://github.com/Tracktion/pluginval) loads the built plugin as a host would and
tests it for stability. It is built from source on demand, so there is nothing to install.

```bash
cmake --build --preset debug --target validate   # builds the plugin and pluginval, then validates
```

To run pluginval together with the unit tests, use the `test` preset (see [Unit Tests](#unit-tests)).

Logs land in `build-debug/pluginval-logs/`. Strictness defaults to 10; use
`-DPLUGINVAL_STRICTNESS=5` (range 1–10) for a faster run, or `-DENABLE_PLUGINVAL=OFF` to skip
pluginval entirely.

The AU test validates the installed component in `~/Library/Audio/Plug-Ins/Components`, since macOS
resolves Audio Units through its registry rather than by path — so it needs `COPY_PLUGIN_AFTER_BUILD`
left on. Steinberg's VST3 conformance validator is off by default, as it pulls ~300MB of SDK for one
extra test; enable it with `-DPLUGINVAL_VST3_VALIDATOR=ON`.

## Plugin Locations

### Build Artifacts

**Debug Build:**
- Standalone: `build-debug/JuceStarter_artefacts/Debug/Standalone/JuceStarter.app`
- AU: `build-debug/JuceStarter_artefacts/Debug/AU/JuceStarter.component`
- VST3: `build-debug/JuceStarter_artefacts/Debug/VST3/JuceStarter.vst3`

**Release Build:**
- Standalone: `build-release/JuceStarter_artefacts/Release/Standalone/JuceStarter.app`
- AU: `build-release/JuceStarter_artefacts/Release/AU/JuceStarter.component`
- VST3: `build-release/JuceStarter_artefacts/Release/VST3/JuceStarter.vst3`

### System Installation (if `COPY_PLUGIN_AFTER_BUILD` is enabled)

**macOS:**
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/JuceStarter.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/JuceStarter.component`


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
