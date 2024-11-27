/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"
#include "animator.h"

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PropertiesDialog)
{
    ui->setupUi(this);
    ui->hidden_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->hidden_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->visible_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->visible_view->setSelectionBehavior(QAbstractItemView::SelectRows);

    setWindowTitle("Properties");

    connect(ui->hide_btn, &QPushButton::clicked, this, [this](){
        QList<QListWidgetItem*> selected_items = ui->visible_view->selectedItems();
        if (selected_items.size() < 1) return;
        int row = ui->visible_view->row(selected_items[0]);
        QListWidgetItem* item = ui->visible_view->takeItem(row);
        QString qname = item->text();
        ui->hidden_view->addItem(item);
        emit PropertyChanged(*properties_[qname], true);
    });

    connect(ui->show_btn, &QPushButton::clicked, this, [this](){
        // Check which item is selected in the Hidden View,
        // then get the corresponding property and emit the changed signal.
        // Also add that item to the Visible View.
        QList<QListWidgetItem*> selected_items = ui->hidden_view->selectedItems();
        if (selected_items.size() < 1) return;
        int row = ui->hidden_view->row(selected_items[0]);
        QListWidgetItem* item = ui->hidden_view->takeItem(row);
        QString qname = item->text();
        ui->visible_view->addItem(item);
        emit PropertyChanged(*properties_[qname], false);
    });
}

PropertiesDialog::~PropertiesDialog() {
    delete ui;
}

int PropertiesDialog::exec(const ObjectWithProperties& properties) {
    ui->visible_view->clear();
    ui->hidden_view->clear();
    properties_.clear();

    for (auto& propname : properties.GetProperties()) {
        auto prop = properties.GetProperty(propname);
        assert(prop != nullptr);
        QString qname = QString::fromStdString(propname);

        // If the property is a vec3property (is this needed?)
        if (Vec3Property* vec3_prop = dynamic_cast<Vec3Property*>(prop)) {
            Property* inner_prop = &(vec3_prop->GetPropertyX());
            QString inner_name = qname + QString::fromStdString("X");
            if (inner_prop->IsHidden()) ui->hidden_view->addItem(inner_name);
            else ui->visible_view->addItem(inner_name);
            properties_[inner_name] = inner_prop;

            inner_prop = &(vec3_prop->GetPropertyY());
            inner_name = qname + QString::fromStdString("Y");
            if (inner_prop->IsHidden()) ui->hidden_view->addItem(inner_name);
            else ui->visible_view->addItem(inner_name);
            properties_[inner_name] = inner_prop;

            inner_prop = &(vec3_prop->GetPropertyZ());
            inner_name = qname + QString::fromStdString("Z");
            if (inner_prop->IsHidden()) ui->hidden_view->addItem(inner_name);
            else ui->visible_view->addItem(inner_name);
            properties_[inner_name] = inner_prop;
        } else {
            if (prop->IsHidden()) ui->hidden_view->addItem(qname);
            else ui->visible_view->addItem(qname);
            properties_[qname] = prop;
        }
    }

    return QDialog::exec();
}
