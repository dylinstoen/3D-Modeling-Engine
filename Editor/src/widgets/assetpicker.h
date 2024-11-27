/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QASSETPICKER_H
#define QASSETPICKER_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <widgets/inspectablewidget.h>
#include <resource/asset.h>

class AssetPicker : public QWidget, public InspectableWidget
{
    Q_OBJECT
public:
    Signal1<Asset*> Changed;
    Signal2<AssetType, AssetPicker&> AssetPickerRequested;

    AssetPicker(const std::string& text, AssetType type, QWidget *parent = Q_NULLPTR);

public slots:
    void OnSetValue(Asset*);
    void OnAssetPicked(Asset* asset);
    void OnAssetDeleted();
    void OnAssetNameChanged(std::string name);
protected:
    QHBoxLayout* layout_;
    QLineEdit* display_;
    QPushButton* dialog_btn_;
    AssetType type_;
    Asset* asset_;
};

#endif // QASSETPICKER_H
