/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "qassetwidgetitem.h"
#include <inspector/inspector.h>
#include <assets/assetbrowser.h>

QAssetWidgetItem::QAssetWidgetItem(Asset& asset) :
    QListWidgetItem(),
    InspectableItem(),
    asset_(&asset)
{
    QString qname = QString::fromStdString(asset.GetName());
    setText(qname);
    setIcon(AssetBrowser::GetIcon(asset.GetType()));
    setFlags(flags() | Qt::ItemIsEditable);

    // Look inside the asset's list of properties and add the corresponding ones
    props_group = new QGroupBox();
    props_group->setTitle(qname);
    QFormLayout* props_layout = new QFormLayout();
    props_group->setLayout(props_layout);
    layout_->addWidget(props_group);
    for (auto& property : asset.GetProperties()) Inspector::AddProperty(*props_layout, property, asset.GetProperty(property), *this);

    // Last thing in the layout and it expands to fill up the bottom
    QGroupBox* filler_group = new QGroupBox();
    filler_group->setFlat(true);
    QVBoxLayout* filler_layout = new QVBoxLayout();
    filler_layout->addStretch(); // Expands to push the button up to the top of the layout
    filler_group->setLayout(filler_layout);
    layout_->addWidget(filler_group);

    asset.NameChanged.Connect(this, &QAssetWidgetItem::OnNameChanged);
}

void QAssetWidgetItem::OnNameChanged(std::string name) {
    props_group->setTitle(QString::fromStdString(name));
    setText(QString::fromStdString(name));
}
