# fooyin Copy Paste Artwork

A fooyin plugin that adds **Copy artwork** and **Paste artwork** to the track right-click menu, using the system clipboard:

- **Copy artwork** puts a track's embedded front cover on the clipboard, ready to paste into a browser, chat app, image editor, office document, etc.
- **Paste artwork** embeds the image currently on the clipboard as the front cover of every selected track. The image can come from anywhere: "Copy image" in a browser, an image viewer, a screenshot tool, a file manager, or **Copy artwork** in fooyin itself.

**Version 0.1.1. Supported: fooyin 0.14.1** on Linux (Flatpak) and Windows x64. This is an independent GPL extension, not an official fooyin release.

## Use

1. Copy an image: right-click a track that has artwork > **Tagging > Copy artwork**, or copy an image in any other app
2. Select the tracks that need the artwork (whole albums work fine)
3. Right-click > **Tagging > Paste artwork**

**Paste artwork** is enabled whenever the clipboard holds an image and all selected tracks are writable.

### Details

- **Image quality:** JPEG and PNG data are embedded byte-for-byte. Other formats (bitmaps, WebP, ...) are converted to JPEG (quality 95), or PNG if they have transparency.
- **What gets copied:** covers go on the clipboard in their original format plus a lossless PNG, so apps that only take PNG still work. Pasting them back into fooyin uses the original bytes.
- **Copied files:** image files copied in a file manager can be pasted too. Under Flatpak, fooyin can only read folders it has access to (by default Music, Downloads and removable media). Images copied from inside an app are not affected.
- **Clipboard lifetime (Linux):** a cover copied from fooyin is served by fooyin itself, so it disappears from the clipboard when fooyin closes unless a clipboard manager (e.g. Klipper) is running.

## Build from source (Linux)

Requirements:
- fooyin installed via Flatpak
- `org.kde.Sdk//6.11` Flatpak runtime
- fooyin v0.14.1 source tree (expected at `../fooyin`; override with `FOOYIN_SOURCE=/path/to/fooyin`)

```bash
git clone --branch v0.14.1 --depth 1 https://github.com/fooyin/fooyin.git ../fooyin
./build.sh
./install.sh   # close fooyin first; the previous plugin is kept as fyplugin_artwork.so.bak
```

## Build from source (Windows)

Requirements:
- Visual Studio 2022 C++ Build Tools
- Qt 6.12.0 MSVC2022 x64 SDK
- Python 3, CMake 3.25+, Ninja
- Installed fooyin 0.14.1 runtime
- fooyin v0.14.1 source

From an x64 Native Tools Command Prompt:

```powershell
.\build.ps1 -QtDirectory 'C:\Qt\6.12.0\msvc2022_64' -FooyinSource '..\fooyin'
```

## Changelog

- **0.1.1:** Copy and Paste now use the system clipboard instead of an internal fooyin-only buffer. Paste accepts images from any app or copied image files.
- **0.1.0:** First release: copy and paste the front cover between tracks inside fooyin.

## Credits and license

By **Gene Dupre**. Licensed under **GPL-3.0-or-later**; see [LICENSE](LICENSE).
