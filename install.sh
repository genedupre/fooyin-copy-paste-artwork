#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PLUGIN="$SCRIPT_DIR/build/fyplugin_artwork.so"

if [ ! -f "$PLUGIN" ]; then
    echo "Plugin not built yet. Run ./build.sh first."
    exit 1
fi

if flatpak list --app 2>/dev/null | grep -q org.fooyin.fooyin; then
    PLUGIN_DIR="$HOME/.var/app/org.fooyin.fooyin/.local/lib/fooyin/plugins"
else
    PLUGIN_DIR="$HOME/.local/lib/fooyin/plugins"
fi

mkdir -p "$PLUGIN_DIR"

if [ -f "$PLUGIN_DIR/fyplugin_artwork.so" ]; then
    cp "$PLUGIN_DIR/fyplugin_artwork.so" "$PLUGIN_DIR/fyplugin_artwork.so.bak"
    echo "Backed up previous plugin to fyplugin_artwork.so.bak"
fi

cp "$PLUGIN" "$PLUGIN_DIR/fyplugin_artwork.so"
echo "Installed to $PLUGIN_DIR/fyplugin_artwork.so"
echo "Restart fooyin to load the new plugin."
