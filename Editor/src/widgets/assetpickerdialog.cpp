/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "assetpickerdialog.h"
#include <assets/qassetwidgetitem.h>

AssetPickerDialog::AssetPickerDialog(QWidget* parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
    list_(new QListWidget())
{
    setWindowTitle("Select Asset");
    setMinimumWidth(512);
    setMinimumHeight(512);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->addWidget(list_);
    setLayout(layout);

    connect(list_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        selection_ = item;
        accept();
    });
}
