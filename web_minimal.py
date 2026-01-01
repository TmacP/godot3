# SCons profile: minimal Web (JavaScript/WebAssembly) export template.
# Usage:
#   scons -j8 p=javascript profile=web_minimal
#
# Goal: smallest practical starting point for a simple 2D GDScript game.
# Add features back by enabling modules/flags as needed.

# Build an export template (not the editor).
tools = "no"
target = "release"

# Prefer smaller binaries.
production = "yes"
optimize = "size"
lto = "full"

# 2D-only.
disable_3d = "yes"

# Optional extra trimming.
# NOTE: For a "basic UI" (typical Control/Container usage), keep this OFF.
# Turning it on removes a lot of UI classes (e.g. PopupMenu, RichTextLabel,
# FileDialog, several Containers...), and will commonly break menus.
disable_advanced_gui = "no"

# Remove deprecated APIs (can break older projects relying on them).
deprecated = "no"

# Extra trimming.
minizip = "yes"
javascript_eval = "yes"
threads_enabled = "no"
gdnative_enabled = "no"

# Start from zero modules; allowlist only what we need.
modules_enabled_by_default = "no"

# Minimal scripting + font rasterization (most UIs need fonts).
module_gdscript_enabled = "yes"
module_freetype_enabled = "yes"

# Additional modules: enable only if needed.
# (See `scons --help` for full list of modules. Most are disabled by default here.)
# Examples:
#   module_regex_enabled = "yes"        # If game uses Regex
#   module_enet_enabled = "yes"         # If game uses networking
#   module_opensimplex_enabled = "yes"  # If game uses procedural noise
