/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "lineedit.h"

LineEdit::LineEdit(const std::string& value, QWidget *parent) :
    QLineEdit(parent)
{
    // Forward the Qt Signal
    connect(this, &QLineEdit::textChanged, this, [this](const QString& text) {
        Changed.Emit(text.toStdString());
    });

    OnSetValue(value);
}

void LineEdit::OnSetValue(std::string value) {
    setText(QString::fromStdString(value));
}
