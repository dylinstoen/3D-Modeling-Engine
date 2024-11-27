/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "filterdialog.h"
#include "ui_filterdialog.h"

FilterDialog::FilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterDialog)
{
    ui->setupUi(this);

    connect(ui->a_spinbox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this]() {
        a_ = ui->a_spinbox->value();
    });

    connect(ui->it_spinbox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this]() {
        iterations_ = ui->it_spinbox->value();
    });

    a_ = ui->a_spinbox->value();
    iterations_ = ui->it_spinbox->value();
}

FilterDialog::~FilterDialog() {
    delete ui;
}

int FilterDialog::exec() {
    return QDialog::exec();
}
