# fooyin Copy Paste Artwork

A fooyin plugin that adds **Copy artwork** and **Paste artwork** actions to the right-click context menu. Copy the embedded front cover from one track, then paste it onto any number of other tracks.

**Supported: fooyin 0.14.1.** Linux (Flatpak) and Windows x64. This is an independent GPL extension, not an official fooyin release.

## Use

1. Right-click a track that has artwork > **Tagging > Copy artwork**
2. Select the tracks that need the same artwork
3. Right-click > **Tagging > Paste artwork**

The front cover from the source track is embedded into all selected target tracks. The artwork stays in memory until you copy a different one or close fooyin.

## Build from source (Linux)

Requirements:
- fooyin installed via Flatpak
- `org.kde.Sdk//6.11` Flatpak runtime
- fooyin v0.14.1 source tree

```bash
git clone --branch v0.14.1 --depth 1 https://github.com/fooyin/fooyin.git ../fooyin
./build.sh
./install.sh
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

## Credits and license

By **Gene Dupre**. Licensed under **GPL-3.0-or-later**; see [LICENSE](LICENSE).
