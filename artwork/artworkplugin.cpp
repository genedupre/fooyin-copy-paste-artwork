// SPDX-License-Identifier: GPL-3.0-or-later
#include "artworkplugin.h"
#include <core/engine/audioloader.h>
#include <core/library/musiclibrary.h>
#include <core/plugins/coreplugincontext.h>
#include <gui/guiconstants.h>
#include <gui/plugins/guiplugincontext.h>
#include <gui/trackselectioncontroller.h>
#include <utils/utils.h>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QUrl>

#include <optional>

using namespace Qt::StringLiterals;
namespace Fooyin::CopyPasteArtwork {
namespace {
constexpr qint64 MaxImageFileSize = 64LL * 1024 * 1024;

QWidget* mainWindow()
{
    return static_cast<QWidget*>(Utils::getMainWindow());
}

QString detectMimeType(const QByteArray& data)
{
    return QMimeDatabase{}.mimeTypeForData(data).name();
}

bool isEmbeddableType(const QString& mimeType)
{
    return mimeType == "image/jpeg"_L1 || mimeType == "image/png"_L1;
}

QByteArray encodeImage(const QImage& image, const char* format, int quality = -1)
{
    QByteArray bytes;
    QBuffer buffer{&bytes};
    buffer.open(QIODevice::WriteOnly);
    if(!image.save(&buffer, format, quality)) {
        return {};
    }
    return bytes;
}

// Encode a decoded image for embedding: PNG if it has transparency, otherwise high-quality JPEG.
std::optional<CoverImage> coverFromImage(const QImage& image)
{
    if(image.isNull()) {
        return {};
    }
    const bool png = image.hasAlphaChannel();
    QByteArray bytes = png ? encodeImage(image, "PNG") : encodeImage(image, "JPEG", 95);
    if(bytes.isEmpty()) {
        return {};
    }
    return CoverImage{.mimeType = png ? u"image/png"_s : u"image/jpeg"_s, .data = std::move(bytes)};
}

// Use encoded bytes as-is when they are JPEG/PNG, otherwise decode and re-encode.
std::optional<CoverImage> coverFromBytes(const QByteArray& data)
{
    if(data.isEmpty()) {
        return {};
    }
    const QString mimeType = detectMimeType(data);
    if(isEmbeddableType(mimeType)) {
        if(QImage::fromData(data).isNull()) {
            return {};
        }
        return CoverImage{.mimeType = mimeType, .data = data};
    }
    return coverFromImage(QImage::fromData(data));
}

bool isImageFile(const QUrl& url)
{
    return url.isLocalFile()
        && QMimeDatabase{}.mimeTypeForFile(url.toLocalFile(), QMimeDatabase::MatchExtension).name().startsWith("image/"_L1);
}

QStringList encodedImageFormats(const QMimeData* mime)
{
    QStringList formats;
    for(const QString& format : mime->formats()) {
        if(format == "image/jpeg"_L1 || format == "image/jpg"_L1 || format == "image/png"_L1) {
            formats.append(format);
        }
    }
    return formats;
}

// Cheap check (format list only, no data transfer) used to enable the Paste action.
bool clipboardHasImage()
{
    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if(!mime) {
        return false;
    }
    if(mime->hasImage()) {
        return true;
    }
    for(const QString& format : mime->formats()) {
        if(format.startsWith("image/"_L1)) {
            return true;
        }
    }
    return mime->hasUrls() && std::ranges::any_of(mime->urls(), isImageFile);
}

std::optional<CoverImage> coverFromClipboard()
{
    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if(!mime) {
        return {};
    }

    // 1. Encoded JPEG/PNG bytes, in the order the source app offers them (its preferred format first).
    for(const QString& format : encodedImageFormats(mime)) {
        if(auto cover = coverFromBytes(mime->data(format))) {
            return cover;
        }
    }

    // 2. Image files copied in a file manager: embed the original file contents.
    if(mime->hasUrls()) {
        for(const QUrl& url : mime->urls()) {
            if(!isImageFile(url)) {
                continue;
            }
            QFile file{url.toLocalFile()};
            if(file.size() > MaxImageFileSize || !file.open(QIODevice::ReadOnly)) {
                continue;
            }
            if(auto cover = coverFromBytes(file.readAll())) {
                return cover;
            }
        }
    }

    // 3. Any other image the platform can hand us (bitmap, webp, ...).
    if(mime->hasImage()) {
        if(auto cover = coverFromImage(qvariant_cast<QImage>(mime->imageData()))) {
            return cover;
        }
    }
    for(const QString& format : mime->formats()) {
        if(format.startsWith("image/"_L1)) {
            if(auto cover = coverFromImage(QImage::fromData(mime->data(format)))) {
                return cover;
            }
        }
    }

    return {};
}

void copyToClipboard(const QByteArray& data)
{
    auto* mime = new QMimeData;

    // Original bytes under their real type, so apps that accept it get the untouched file.
    const QString mimeType = detectMimeType(data);
    if(mimeType.startsWith("image/"_L1)) {
        mime->setData(mimeType, data);
    }

    const QImage image = QImage::fromData(data);
    if(!image.isNull()) {
        // Lossless PNG for apps that only take PNG (most of them on Linux).
        if(mimeType != "image/png"_L1) {
            const QByteArray png = encodeImage(image, "PNG");
            if(!png.isEmpty()) {
                mime->setData(u"image/png"_s, png);
            }
        }
        // Native bitmap formats (CF_DIB on Windows, image/bmp etc. on X11). Skipped on Wayland, where
        // Qt would re-encode every image/* type from this QImage instead of using the bytes above.
        if(QGuiApplication::platformName() != "wayland"_L1) {
            mime->setImageData(image);
        }
    }

    QGuiApplication::clipboard()->setMimeData(mime);
}
} // namespace

void ArtworkPlugin::initialise(const CorePluginContext& context)
{
    m_library = context.library;
    m_loader  = context.audioLoader;
}

void ArtworkPlugin::initialise(const GuiPluginContext& context)
{
    m_trackSelection = context.trackSelection;

    m_trackSelection->registerTrackContextAction(
        this, TrackContextMenuArea::Track, Constants::Menus::Context::Tagging, "CopyPasteArtwork.Copy",
        tr("Copy artwork"), [this](QMenu* menu, const TrackSelection& selection) {
            if(selection.tracks.empty()) return;
            auto* action = menu->addAction(tr("Copy artwork"));
            QObject::connect(action, &QAction::triggered, this, [this, tracks = selection.tracks] {
                const auto data = m_loader->readTrackCover(tracks.front(), Track::Cover::Front);
                if(data.isEmpty()) {
                    QMessageBox::information(mainWindow(), tr("No artwork"),
                                             tr("The selected track has no embedded front cover."));
                    return;
                }
                copyToClipboard(data);
            });
        });

    m_trackSelection->registerTrackContextAction(
        this, TrackContextMenuArea::Track, Constants::Menus::Context::Tagging, "CopyPasteArtwork.Paste",
        tr("Paste artwork"), [this](QMenu* menu, const TrackSelection& selection) {
            if(selection.tracks.empty()) return;
            bool canWrite = std::ranges::all_of(selection.tracks, [this](const Track& track) {
                return track.isMetadataEditable(m_loader->canWriteMetadata(track));
            });
            auto* action = menu->addAction(tr("Paste artwork"));
            action->setEnabled(canWrite && clipboardHasImage());
            QObject::connect(action, &QAction::triggered, this, [this, tracks = selection.tracks] {
                auto cover = coverFromClipboard();
                if(!cover) {
                    QMessageBox::information(mainWindow(), tr("No image"),
                                             tr("The clipboard does not contain an image that can be used as artwork."));
                    return;
                }
                TrackCovers covers;
                covers[Track::Cover::Front] = std::move(*cover);
                TrackCoverData coverData;
                coverData.tracks    = tracks;
                coverData.coverData = covers;
                m_library->writeTrackCovers(coverData);
            });
        });
}
} // namespace Fooyin::CopyPasteArtwork
