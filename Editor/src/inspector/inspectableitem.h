/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef INSPECTABLEITEM_H
#define INSPECTABLEITEM_H

#include <animator.h>
#include <resources.h>
#include <QWidget>
#include <QVBoxLayout>
#include <widgets/assetpicker.h>

class InspectableItem {
public:
    InspectableItem() :
        widget_(new QWidget()),
        layout_(new QVBoxLayout()),
        internal_(false)
    {
        widget_->setLayout(layout_);
    }

    virtual QWidget* GetWidget() { return widget_; }
    virtual std::string GetName() = 0;

    void SetInternal() { internal_ = true; }
    Signal0<> RedrawRequested;
    void OnRedrawRequested() { if (!internal_) RedrawRequested.Emit(); }

    // This is specific to asset picker so can there be a way to not put this here?
    Signal2<AssetType, AssetPicker&> AssetPickerRequested;
    void OnAssetPickerRequested(AssetType type, AssetPicker& requester) { AssetPickerRequested.Emit(type, requester); }
protected:
    QWidget* widget_;
    QVBoxLayout* layout_;
    bool internal_;
};

#endif // INSPECTABLEITEM_H
