/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "intedit.h"

QIntValidator IntEdit::validator_;

IntEdit::IntEdit(int value, bool unsigned_only, QWidget *parent) :
    QLineEdit(parent),
    value_(value),
    unsigned_only_(unsigned_only)
{
    // Validator ensures that the QLineEdit will only accept text that represents a integer
    if (unsigned_only) validator_.setBottom(0);
    setValidator(&validator_);
    setMinimumWidth(35); // Width of the QLineEdit
    // Forward the Qt Signal
    connect(this, &QLineEdit::textChanged, this, &IntEdit::OnChanged);

    OnSetValue(value);
}

void IntEdit::OnChanged(const QString &text) {
    Changed.Emit(text.toInt());
}

void IntEdit::OnSetValue(int value) {
    if (unsigned_only_ && (value < 0)) value = 0;
    value_ = value;
    int cursor_pos = cursorPosition();
    setText(QString::number(value_));
    setCursorPosition(cursor_pos);
    RedrawRequested.Emit();
}
