/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "colorpicker.h"

QColor FromVec4(glm::vec4 value) {
    return QColor(value.r * 255, value.g * 255, value.b * 255, value.a * 255);
}

ColorPicker::ColorPicker(bool use_alpha, const glm::vec4& value, QWidget *parent) :
    QWidget(parent),
    layout_(new QHBoxLayout()),
    display_(new ColorPreview()),
    dialog_(new ColorDialog()),
    dialog_btn_(new QPushButton()),
    use_alpha_(use_alpha)
{
    if (use_alpha) display_->setDisplayMode(ColorPreview::DisplayMode::AllAlpha);
    else display_->setDisplayMode(ColorPreview::DisplayMode::NoAlpha);

    setLayout(layout_);
    layout_->setMargin(0);
    layout_->addWidget(display_);
    layout_->addWidget(dialog_btn_);
    display_->setColor(FromVec4(value));
    dialog_->setColor(FromVec4(value));
    dialog_->setPreviewDisplayMode(ColorPreview::DisplayMode::AllAlpha);
    dialog_btn_->setMaximumWidth(25);
    dialog_btn_->setIcon(QIcon(":/images/icons/edit.png"));
    dialog_btn_->setFlat(true);

    connect(dialog_, &ColorDialog::colorChanged, this, [&, this](QColor color) {
        Changed.Emit(glm::vec4(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0, color.alpha() / 255.0));
    });
    connect(dialog_btn_, &QPushButton::clicked, this, [&, this] {
        QColor originalcolor = dialog_->color();
        if(dialog_->exec()) {
            QColor color = dialog_->color();
            Changed.Emit(glm::vec4(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0, color.alpha() / 255.0));
        } else {
            Changed.Emit(glm::vec4(originalcolor.red() / 255.0, originalcolor.green() / 255.0, originalcolor.blue() / 255.0, originalcolor.alpha() / 255.0));
            dialog_->blockSignals(true);
            dialog_->setColor(originalcolor);
            dialog_->blockSignals(false);
        }
    });
}

void ColorPicker::OnSetValue(glm::vec4 value) {
    if (!use_alpha_) value.a = 0;
    display_->setColor(FromVec4(value));
    RedrawRequested.Emit();
}
