/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ASSETBROWSER_H
#define ASSETBROWSER_H

#include <animator.h>
#include <QListWidget>
#include <widgets/assetpickerdialog.h>
#include <inspector/inspectableitem.h>
#include <resource/asset.h>
#include <scene/scene.h>
#include <assets/qassetwidgetitem.h>
#include <QIcon>

class AssetBrowser : public QListWidget {
    Q_OBJECT

public:
    AssetBrowser(QWidget* parent = nullptr);
    void SetScene(Scene& scene);
    static QIcon GetIcon(AssetType type);

public slots:
    void OnItemRenamed(QListWidgetItem* item);
    void OnAssetCreated(Asset& asset);
    void OnAssetDeleted(uint64_t asset_id);
    void OnAssetPicker(AssetType type, AssetPicker& requester); // Maybe this should be a delegate instead?
    void OnChanged();

    void keyPressEvent(QKeyEvent *event) override;

    void DeleteAsset();

protected:
    std::unordered_map<uint64_t, QAssetWidgetItem*> asset_widgets_;
    AssetManager* asset_manager_;
    QAssetWidgetItem* CreateWidgetItem(Asset& asset);
    AssetPickerDialog* dialog_;
    QMenu* context_menu_;

signals:
    void InspectableSelected(InspectableItem* object_widget);
    void InspectableAdded(InspectableItem& object_widget);
    void InspectableRemoved(InspectableItem& object_widget);
    void RedrawRequested();
};

#endif // ASSETBROWSER_H
