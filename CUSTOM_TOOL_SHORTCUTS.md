# Custom Tool Menu Shortcuts

This Godot 3.6 fork adds support for binding shortcuts to custom tool menu items added by editor plugins.

## How to Use

Editor plugins can now specify a `ShortCut` when adding tool menu items. This makes the action bindable in Editor Settings under **Editor > Editor Settings > Shortcuts**.

### Basic Example

```gdscript
tool
extends EditorPlugin

func _enter_tree():
    # Create a shortcut for your tool action
    var shortcut = ShortCut.new()
    var input_event = InputEventKey.new()
    input_event.scancode = KEY_F9  # F9 key
    input_event.control = true      # Ctrl modifier
    shortcut.set_shortcut(input_event)
    
    # Add tool menu item with shortcut
    add_tool_menu_item("My Custom Export", self, "_my_export_callback", null, shortcut)

func _my_export_callback():
    print("Export triggered!")
    # Your export logic here

func _exit_tree():
    remove_tool_menu_item("My Custom Export")
```

### Poki SDK Plugin Example

For the Poki SDK plugin, you can modify your plugin script to add a shortcut for the "Run poki" export:

```gdscript
tool
extends EditorPlugin

func _enter_tree():
    # Set up the Poki export preset (your existing code)
    var cfg = ConfigFile.new()
    cfg.load("res://export_presets.cfg")
    
    if(self.is_poki_added(cfg)):
        print("Poki export already added")
    else:
        add_poki_export(cfg)
        cfg.save("res://export_presets.cfg")
    
    add_autoload_singleton("PokiSDK", "res://addons/poki-sdk/pokisdk.gd")
    
    # Create a shortcut for the Poki export (Ctrl+Shift+F9 for example)
    var poki_shortcut = ShortCut.new()
    var poki_key = InputEventKey.new()
    poki_key.scancode = KEY_F9
    poki_key.control = true
    poki_key.shift = true
    poki_shortcut.set_shortcut(poki_key)
    
    # Add the tool menu item with the shortcut
    add_tool_menu_item("Run poki", self, "_run_poki_export", null, poki_shortcut)

func _run_poki_export():
    # Your Poki export logic here
    print("Running Poki export...")
    # Example: trigger the export dialog or directly export
    # EditorNode.get_singleton().export_preset("Poki", debug_mode)

func _exit_tree():
    remove_autoload_singleton("PokiSDK")
    remove_tool_menu_item("Run poki")

# Your existing helper functions...
func is_poki_added(cfg:ConfigFile):
    # ... existing code ...
    pass

func add_poki_export(cfg:ConfigFile):
    # ... existing code ...
    pass
```

## How Users Can Customize the Shortcut

After you rebuild and install your custom Godot editor:

1. Open your Godot project
2. Go to **Editor > Editor Settings**
3. Navigate to the **Shortcuts** section
4. Search for your tool menu item name (e.g., "Run poki")
5. Click on the shortcut and press your desired key combination
6. The new shortcut is saved automatically

## Technical Details

The implementation adds an optional `shortcut` parameter to:
- `EditorNode::add_tool_menu_item()`
- `EditorPlugin::add_tool_menu_item()`

When a shortcut is provided, the menu item becomes bindable through the editor's shortcut system, allowing users to customize the keybinding via Editor Settings.

## Building

After making changes, rebuild your Godot editor:

```sh
scons -j8 platform=osx arch=arm64 target=release_debug
```

Then package it as an app (if needed):

```sh
rm -rf bin/Godot.app
cp -R misc/dist/osx_tools.app bin/Godot.app
mkdir -p bin/Godot.app/Contents/MacOS
cp -f bin/godot.osx.opt.tools.arm64 bin/Godot.app/Contents/MacOS/Godot
chmod +x bin/Godot.app/Contents/MacOS/Godot
```
