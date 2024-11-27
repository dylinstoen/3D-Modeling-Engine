/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "assetpicker.h"

AssetPicker::AssetPicker(const std::string& text, AssetType type, QWidget *parent) :
    QWidget(parent),
    layout_(new QHBoxLayout()),
    display_(new QLineEdit()),
    dialog_btn_(new QPushButton()),
    type_(type),
    asset_(nullptr)
{
    setLayout(layout_);
    layout_->setMargin(0);
    layout_->addWidget(display_);
    layout_->addWidget(dialog_btn_);
    display_->setDisabled(true);
    display_->setText(QString::fromStdString(text));

    // dialog_btn_->setFlat(true);
    dialog_btn_->setText(QString("..."));
    dialog_btn_->setMaximumWidth(25);

    connect(dialog_btn_, &QPushButton::clicked, this, [this]{ AssetPickerRequested.Emit(type_, *this); });
}

void AssetPicker::OnAssetPicked(Asset* asset) { Changed.Emit(asset); }
void AssetPicker::OnAssetDeleted() {
    if (asset_ != nullptr) {
        asset_->NameChanged.Disconnect(this, &AssetPicker::OnAssetNameChanged);
        asset_->Deleted.Disconnect(this, &AssetPicker::OnAssetDeleted);
    }
    asset_ = nullptr;
    display_->setText(tr(""));
    RedrawRequested.Emit();
}

void AssetPicker::OnSetValue(Asset* asset) {
    std::string text("");
	// BUG: If asset_ is not nullptr but it is valid, then we've got a problem
    if (asset_ != nullptr) {
        asset_->NameChanged.Disconnect(this, &AssetPicker::OnAssetNameChanged);
        asset_->Deleted.Disconnect(this, &AssetPicker::OnAssetDeleted);
    }
    if (asset != nullptr) {
        text = asset->GetName();
        asset->NameChanged.Connect(this, &AssetPicker::OnAssetNameChanged);
        asset->Deleted.Connect(this, &AssetPicker::OnAssetDeleted);
        asset_ = asset;
    }
    display_->setText(QString::fromStdString(text));
    RedrawRequested.Emit();
}

void AssetPicker::OnAssetNameChanged(std::string name) {
    display_->setText(QString::fromStdString(name));
    RedrawRequested.Emit();
}


