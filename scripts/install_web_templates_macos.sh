#!/usr/bin/env bash
set -euo pipefail

# Installs locally-built Web export templates into the Godot editor templates folder on macOS.
# Godot expects:
#   ~/Library/Application Support/Godot/templates/<VERSION_FULL_CONFIG>/webassembly_{release,debug}.zip

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

version_full_config="$(
python3 - <<'PY'
import re
from pathlib import Path
text = Path('core/version_generated.gen.h').read_text(encoding='utf-8')

def grab(pattern: str) -> str:
    m = re.search(pattern, text)
    if not m:
        raise SystemExit(f"Could not find pattern: {pattern}")
    return m.group(1)

major = int(grab(r"#define VERSION_MAJOR\s+(\d+)"))
minor = int(grab(r"#define VERSION_MINOR\s+(\d+)"))
patch = int(grab(r"#define VERSION_PATCH\s+(\d+)"))
status = grab(r"#define VERSION_STATUS\s+\"([^\"]+)\"")
module_config = grab(r"#define VERSION_MODULE_CONFIG\s+\"([^\"]*)\"")

print(f"{major}.{minor}.{patch}.{status}{module_config}")
PY
)"

templates_base="$HOME/Library/Application Support/Godot/templates"
dest_dir="$templates_base/$version_full_config"

src_release="bin/godot.javascript.opt.zip"
if [[ ! -f "$src_release" ]]; then
  echo "Missing $src_release"
  echo "Build it with: scons -j8 p=javascript profile=web_minimal"
  exit 1
fi

mkdir -p "$dest_dir"
cp -f "$src_release" "$dest_dir/webassembly_release.zip"

echo "Installed webassembly_release.zip -> $dest_dir"

# Try to find a debug template zip. If none exists, reuse the release zip so the editor can export in Debug too.
shopt -s nullglob
candidates=(bin/godot.javascript.*debug*.zip)
shopt -u nullglob

if (( ${#candidates[@]} > 0 )) && [[ -f "${candidates[0]}" ]]; then
  cp -f "${candidates[0]}" "$dest_dir/webassembly_debug.zip"
  echo "Installed webassembly_debug.zip from ${candidates[0]}"
else
  cp -f "$src_release" "$dest_dir/webassembly_debug.zip"
  echo "No debug zip found; copied release as webassembly_debug.zip (ok for testing)."
fi

echo
echo "Done. Restart the editor, then export HTML5/Web."