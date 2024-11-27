/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "combobox.h"

ComboBox::ComboBox(std::vector<std::string> choices, int selected, QWidget *parent) :
    QComboBox(parent)
{
    // Requires casting because currentIndexChanged is an overloaded function
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ComboBox::OnSelected);
    for (auto& choice : choices) insertItem(choices.size(), QString::fromStdString(choice));
    setCurrentIndex(selected);
}

void ComboBox::OnSelected(int selection) { Changed.Emit(selection); }

void ComboBox::SetCurrentIndex(int index) {
    setCurrentIndex(index);
    RedrawRequested.Emit();
}
