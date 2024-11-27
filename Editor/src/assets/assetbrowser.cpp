/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "assetbrowser.h"
#include <QtGlobal>
#include <QKeyEvent>

AssetBrowser::AssetBrowser(QWidget* parent) :
    QListWidget(parent),
    asset_manager_(nullptr),
    dialog_(new AssetPickerDialog()),
    context_menu_(new QMenu(tr("Context Menu"), this))
{
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Hook up the internal signal that gets called to the event handler
    QObject::connect(this, &QListWidget::itemSelectionChanged, this, &AssetBrowser::OnChanged);

    // Handle renaming an object in the asset browser
    QObject::connect(this, &AssetBrowser::itemChanged, this, &AssetBrowser::OnItemRenamed);
}

void AssetBrowser::SetScene(Scene& scene) {
    // Update the state of the widget
    clear(); // Removes all items and selections
    asset_widgets_.clear();

    AssetManager& assets = scene.GetAssetManager();
    auto textures = assets.GetTextures();
    auto cubemaps = assets.GetCubemaps();
    auto materials = assets.GetMaterials();
    auto meshes = assets.GetMeshes();
    auto shader_programs = assets.GetShaderPrograms();
    for (auto& asset : textures) CreateWidgetItem(*asset);
    for (auto& asset : cubemaps) CreateWidgetItem(*asset);
    for (auto& asset : materials) CreateWidgetItem(*asset);
    for (auto& asset : meshes) if (asset) CreateWidgetItem(*asset);
    for (auto& asset : shader_programs) CreateWidgetItem(*asset);
    asset_manager_ = &assets;

    // Connect to the signals sent from the AssetManager so we can update the view
    assets.AssetCreated.Connect(this, &AssetBrowser::OnAssetCreated);
    assets.AssetDeleted.Connect(this, &AssetBrowser::OnAssetDeleted);

    QAction* rename_action = new QAction("Rename", this);
    connect(rename_action, &QAction::triggered, this, [this] {
        editItem(selectedItems()[0]);
    });
    QAction* del_action = new QAction("Delete", this);
    connect(del_action, &QAction::triggered, this, [this]{
        DeleteAsset();
    });
    context_menu_->addAction(rename_action);
    context_menu_->addAction(del_action);

    // Show the context menu at the position requested only if there's an item selected
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (selectedItems().size() > 0) context_menu_->exec(mapToGlobal(pos));
    });
}

void AssetBrowser::OnAssetCreated(Asset& asset) {
    CreateWidgetItem(asset);
}

void AssetBrowser::OnAssetDeleted(uint64_t asset_id) {
    if (asset_widgets_.count(asset_id) < 1) return;
    delete takeItem(row(asset_widgets_[asset_id]));
    asset_widgets_.erase(asset_id);
}

void AssetBrowser::OnChanged() {
    QList<QListWidgetItem*> selected = selectedItems();
    // TODO: Handle multiple selection
    // For now, just use the first thing in the selection
    QAssetWidgetItem* inspectable_item = nullptr;
    if (selected.size() > 0) {
        // Safe to dynamic cast to QAssetWidgetItem since we only put those in
        inspectable_item = dynamic_cast<QAssetWidgetItem*>(selected[0]);
    }
    emit InspectableSelected(inspectable_item);
}

QAssetWidgetItem* AssetBrowser::CreateWidgetItem(Asset& asset) {
    if (asset.IsHidden()) return nullptr;
    uint64_t uid = asset.GetUID();
    QAssetWidgetItem* node = new QAssetWidgetItem(asset);
    node->RedrawRequested.Connect(this, &AssetBrowser::RedrawRequested);
    node->AssetPickerRequested.Connect(this, &AssetBrowser::OnAssetPicker);
    asset_widgets_[uid] = node;
    addItem(node);
    emit InspectableAdded(*node);
    return node;
}

void AssetBrowser::OnAssetPicker(AssetType type, AssetPicker &requester) {
    // TODO: This is inefficient as hell, but for now it'll do
    std::vector<QListWidgetItem*> items;
    for (auto& kv : asset_widgets_) {
        if (type == kv.second->GetAsset()->GetType()) {
            Asset* asset = kv.second->GetAsset();
            // Store the asset id into a list widget and label/icon it appropriately to be displayed
            quint64 asset_id = asset->GetUID();
            QListWidgetItem* list_item = new QListWidgetItem();
            list_item->setData(Qt::UserRole, QVariant(asset_id));
            list_item->setText(QString::fromStdString(asset->GetName()));
            list_item->setIcon(AssetBrowser::GetIcon(asset->GetType()));
            items.push_back(list_item);
        }
    }

    dialog_->SetItems(items);
    if (dialog_->exec()) {
        quint64 asset_id = dialog_->GetSelection()->data(Qt::UserRole).toULongLong();
        Asset* asset = asset_widgets_[asset_id]->GetAsset();
        requester.OnAssetPicked(asset);
    }
}

QIcon AssetBrowser::GetIcon(AssetType type) {
    QIcon icon(":/images/icons/unknown.png");
    switch(type) {
        case AssetType::Material:
            icon = QIcon(":/images/icons/material.png");
            break;
        case AssetType::Mesh:
            icon = (QIcon(":/images/icons/mesh.png"));
            break;
        case AssetType::Texture:
            icon = (QIcon(":/images/icons/texture.png"));
            break;
        case AssetType::Cubemap:
            icon = (QIcon(":/images/icons/cubemap.png"));
            break;
        case AssetType::ShaderProgram:
            icon = (QIcon(":/images/icons/shader.png"));
            break;
        case AssetType::RenderableCubemap:
        case AssetType::RenderableTexture:
            assert(false); // Should never happen; Renderable* is an internal asset type
            break;
    }
    return icon;
}

void AssetBrowser::OnItemRenamed(QListWidgetItem* item) {
    QAssetWidgetItem* inspectable_item = dynamic_cast<QAssetWidgetItem*>(item);
    Asset* edited_asset = inspectable_item->GetAsset();
    std::string new_name = item->text().toStdString();
    // Check if new_name already exists
    switch(edited_asset->GetType()) {
        case AssetType::Material: {
            auto materials = asset_manager_->GetMaterials();
            for (auto& asset : materials) {
                // If it does exist, set the name back to what it was before.
                if (asset->GetName() == new_name) {
                    item->setText(QString::fromStdString(edited_asset->GetName()));
                    return;
                }
            }
            break; }
        case AssetType::Mesh: {
            auto meshes = asset_manager_->GetMeshes();
            for (auto& asset : meshes) {
                // If it does exist, set the name back to what it was before.
                if (asset->GetName() == new_name) {
                    item->setText(QString::fromStdString(edited_asset->GetName()));
                    return;
                }
            }
            break; }
        case AssetType::Texture: {
            auto textures = asset_manager_->GetTextures();
            for (auto& asset : textures) {
                // If it does exist, set the name back to what it was before.
                if (asset->GetName() == new_name) {
                    item->setText(QString::fromStdString(edited_asset->GetName()));
                    return;
                }
            }
            break; }
        case AssetType::Cubemap: {
            auto cubemaps = asset_manager_->GetCubemaps();
            for (auto& asset : cubemaps) {
                // If it does exist, set the name back to what it was before.
                if (asset->GetName() == new_name) {
                    item->setText(QString::fromStdString(edited_asset->GetName()));
                    return;
                }
            }
            break; }
        case AssetType::ShaderProgram: {
            auto shader_programs = asset_manager_->GetShaderPrograms();
            for (auto& asset : shader_programs) {
                // If it does exist, set the name back to what it was before.
                if (asset->GetName() == new_name) {
                    item->setText(QString::fromStdString(edited_asset->GetName()));
                    return;
                }
            }
            break; }
        case AssetType::RenderableCubemap:
        case AssetType::RenderableTexture:
            assert(false); // Should never happen; Renderable* is an internal asset type
            break;
    }
    // Else, we're good to change the asset name
    inspectable_item->GetAsset()->SetName(new_name);
}

void AssetBrowser::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        DeleteAsset();
    }
}

void AssetBrowser::DeleteAsset() {
    QList<QListWidgetItem*> selected = selectedItems();
    for (QListWidgetItem* item : selected) {
        QAssetWidgetItem* asset = dynamic_cast<QAssetWidgetItem*>(item);
        if (asset == nullptr) return;
        std::string name = asset->GetAsset()->GetName();
        switch(asset->GetAsset()->GetType()) {
            case AssetType::Material:
                asset_manager_->UnloadMaterial(name);
                break;
            case AssetType::Mesh:
                asset_manager_->UnloadMesh(name);
                break;
            case AssetType::Texture:
                asset_manager_->UnloadTexture(name);
                break;
            case AssetType::Cubemap:
                asset_manager_->UnloadCubemap(name);
                break;
            case AssetType::ShaderProgram:
                asset_manager_->UnloadShaderProgram(name);
                break;
            case AssetType::RenderableCubemap:
            case AssetType::RenderableTexture:
                assert(false); // Should never happen; Renderable* is an internal asset type
                break;
        }
    }
}
