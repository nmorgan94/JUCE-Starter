# PluginTemplate

A JUCE audio plugin template for creating VST3, AU, and Standalone audio plugins.

## Features

- VST3, AU, and Standalone formats
- Modern C++23
- Automatic JUCE dependency management via CPM
- Binary asset support


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
open build-debug/PluginTemplate_artefacts/Debug/Standalone/PluginTemplate.app

# Release version
open build-release/PluginTemplate_artefacts/Release/Standalone/PluginTemplate.app
```

**Build and Run in One Command:**
```bash
cmake --preset release && cmake --build --preset release && open build-release/PluginTemplate_artefacts/Release/Standalone/PluginTemplate.app
```

### What Happens During Build

1. **First time only**: CMake downloads JUCE automatically via CPM
2. **Every build**: Compiles your plugin in the selected configuration
3. **If `COPY_PLUGIN_AFTER_BUILD` is enabled**: Installs plugins to system directories

## Plugin Locations

### Build Artifacts

**Debug Build:**
- Standalone: `build-debug/PluginTemplate_artefacts/Debug/Standalone/PluginTemplate.app`
- AU: `build-debug/PluginTemplate_artefacts/Debug/AU/PluginTemplate.component`
- VST3: `build-debug/PluginTemplate_artefacts/Debug/VST3/PluginTemplate.vst3`

**Release Build:**
- Standalone: `build-release/PluginTemplate_artefacts/Release/Standalone/PluginTemplate.app`
- AU: `build-release/PluginTemplate_artefacts/Release/AU/PluginTemplate.component`
- VST3: `build-release/PluginTemplate_artefacts/Release/VST3/PluginTemplate.vst3`

### System Installation (if `COPY_PLUGIN_AFTER_BUILD` is enabled)

**macOS:**
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/PluginTemplate.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/PluginTemplate.component`


## Customization

Edit `CMakeLists.txt` to customize:

```cmake
project(PluginTemplate)  # Change project name

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME MyCompanyName            # Your company name
    PLUGIN_MANUFACTURER_CODE Mycn         # 4-char manufacturer ID
    PLUGIN_CODE Plgn                      # 4-char unique plugin ID
    VERSION 0.0.0                         # Version number
    FORMATS VST3 AU Standalone            # Plugin formats to build
    # ... other options
)
```

## Project Structure

```
├── Assets/                  # Binary assets (images, audio files, etc.)
├── build-debug/             # Debug build directory (generated)
├── build-release/           # Release build directory (generated)
└── Source/
    ├── PluginEditor.cpp     # GUI implementation
    ├── PluginEditor.h       # GUI header
    ├── PluginProcessor.cpp  # Audio processing
    └── PluginProcessor.h    # Audio processor header
├── CMakeLists.txt           # Build configuration
├── CMakePresets.json        # CMake presets for Debug/Release
```

## Clean Build

```bash
# Remove build directories and rebuild
rm -rf build-debug build-release

# Rebuild Debug
cmake --preset debug
cmake --build --preset debug

# Rebuild Release
cmake --preset release
cmake --build --preset release
```