/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QASSETPICKERDIALOG_H
#define QASSETPICKERDIALOG_H

#include <QDialog>
#include <QListWidget>

class QAssetWidgetItem;

class AssetPickerDialog : public QDialog
{
public:
    AssetPickerDialog(QWidget *parent = nullptr);
    void SetItems(std::vector<QListWidgetItem*>& list) {
        list_->clear();
        for (auto& widget : list) {
            if (widget == nullptr) continue;
            list_->addItem(widget);
        }
    }
    QListWidgetItem* GetSelection() { return selection_; }
private:
    QListWidget* list_;
    QListWidgetItem* selection_;
};

#endif // QASSETPICKERDIALOG_H
