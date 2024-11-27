/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "doubleedit.h"
#include <QLineEdit>

DoubleEdit::DoubleEdit(double value, int precision, QWidget *parent) :
    QDoubleSpinBox(parent),
    precision_(precision)
{
    setButtonSymbols(QAbstractSpinBox::NoButtons);
    setMinimum(std::numeric_limits<float>::lowest());
    setMaximum(std::numeric_limits<float>::max());
    setDecimals(precision);
    setMinimumWidth(35);
    // Forward the Qt Signal
    connect(this, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DoubleEdit::OnChanged);

    OnSetValue(value);
}

void DoubleEdit::OnChanged(double value) {
    Changed.Emit(value);
}

void DoubleEdit::OnSetValue(double value) {
    const QSignalBlocker blocker(this);
    value_ = value;
    setValue(value_);
    RedrawRequested.Emit();
}

QString DoubleEdit::textFromValue(double val) const
{
    QString text = QDoubleSpinBox::textFromValue(val);
    // Strip the trailing zeroes
    int cursor_pos = lineEdit()->cursorPosition();
    text.remove( QRegExp("0+$") ); // Remove any number of trailing 0's
    text.remove( QRegExp("\\.$") ); // If the last character is just a '.' then remove it
    lineEdit()->setCursorPosition(cursor_pos);
    return text;
}
