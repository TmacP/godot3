# Tristen build notes (Godot 3.x fork)

## Custom Features

### Tool Menu Shortcuts
This fork adds support for binding shortcuts to custom tool menu items. See [CUSTOM_TOOL_SHORTCUTS.md](CUSTOM_TOOL_SHORTCUTS.md) for usage instructions and examples.

## Editor (macOS ARM64)
Build the editor you run locally on your Mac:

```sh
scons -j8 platform=osx arch=arm64 target=release_debug
```

# TODO I want to move our godot to my app
## turn godot.osx.opt.tools.arm64 into app move to applications

```sh
scons -j8 platform=osx arch=arm64 target=release_debug

rm -rf bin/Godot.app
cp -R misc/dist/osx_tools.app bin/Godot.app
mkdir -p bin/Godot.app/Contents/MacOS

cp -f bin/godot.osx.opt.tools.arm64 bin/Godot.app/Contents/MacOS/Godot
chmod +x bin/Godot.app/Contents/MacOS/Godot


sudo rm -rf /Applications/Godot.app
sudo ditto bin/Godot.app /Applications/Godot.app

```


## Web export template (minimal 2D)
Build the Web (WASM/JS) export template zip using the profile in `web_minimal.py`:

```sh
scons -j8 p=javascript profile=web_minimal
bash scripts/install_web_templates_macos.sh
```

Restart the editor after installing.