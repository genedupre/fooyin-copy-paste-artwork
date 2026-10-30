// SPDX-License-Identifier: GPL-3.0-or-later
#include "artworkplugin.h"
#include <core/engine/audioloader.h>
#include <core/library/musiclibrary.h>
#include <core/plugins/coreplugincontext.h>
#include <gui/guiconstants.h>
#include <gui/plugins/guiplugincontext.h>
#include <gui/trackselectioncontroller.h>
#include <utils/utils.h>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>

using namespace Qt::StringLiterals;
namespace Fooyin::CopyPasteArtwork {
void ArtworkPlugin::initialise(const CorePluginContext& context)
{
    m_library = context.library;
    m_loader = context.audioLoader;
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
                    QMessageBox::information(static_cast<QWidget*>(Utils::getMainWindow()),
                        tr("No artwork"), tr("The selected track has no embedded front cover."));
                    return;
                }
                m_clipboard.data = data;
                m_clipboard.mimeType = data.startsWith("\x89PNG") ? u"image/png"_s : u"image/jpeg"_s;
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
            action->setEnabled(canWrite && !m_clipboard.data.isEmpty());
            QObject::connect(action, &QAction::triggered, this, [this, tracks = selection.tracks] {
                if(m_clipboard.data.isEmpty()) return;
                TrackCovers covers;
                covers[Track::Cover::Front] = m_clipboard;
                TrackCoverData coverData;
                coverData.tracks = tracks;
                coverData.coverData = covers;
                m_library->writeTrackCovers(coverData);
            });
        });
}
} // namespace Fooyin::CopyPasteArtwork
