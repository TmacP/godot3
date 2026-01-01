#!/bin/bash

set -e

# Prompt for sudo password at the start
sudo -v

echo "Building Godot 3.x for macOS ARM64..."

# Build Godot
echo "Step 1: Building godot.osx.opt.tools.arm64..."
scons -j8 platform=osx arch=arm64 target=release_debug

# Create Godot.app structure
echo "Step 2: Creating Godot.app bundle..."
rm -rf bin/Godot.app
cp -R misc/dist/osx_tools.app bin/Godot.app
mkdir -p bin/Godot.app/Contents/MacOS

# Copy the compiled binary
echo "Step 3: Installing binary into app bundle..."
cp -f bin/godot.osx.opt.tools.arm64 bin/Godot.app/Contents/MacOS/Godot
chmod +x bin/Godot.app/Contents/MacOS/Godot

# Copy to Applications
echo "Step 4: Installing to /Applications..."
sudo rm -rf /Applications/Godot.app
sudo ditto bin/Godot.app /Applications/Godot.app

# Build web export template
echo "Step 5: Building web export template (minimal 2D)..."
scons -j8 p=javascript profile=web_minimal

echo "Step 6: Installing web templates..."
bash scripts/install_web_templates_macos.sh

echo "✓ Build complete! Godot is installed at /Applications/Godot.app"
