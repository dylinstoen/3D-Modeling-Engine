/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QASSETWIDGETITEM_H
#define QASSETWIDGETITEM_H

#include <qtwidgets.h>
#include <inspector/inspectableitem.h>
#include <resource/asset.h>

class QAssetWidgetItem : public QListWidgetItem, public InspectableItem {
public:
    QAssetWidgetItem(Asset& asset);

    virtual std::string GetName() { return name_; }
    AssetType GetType() { return type_; }
    Asset* GetAsset() { return asset_; }
private:
    void OnNameChanged(std::string name);

    std::string name_;
    AssetType type_;
    Asset* asset_;
    QGroupBox* props_group;
};

#endif // QASSETWIDGETITEM_H
