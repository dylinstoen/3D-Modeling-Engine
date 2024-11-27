/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "vec3edit.h"

Vec3Edit::Vec3Edit(glm::vec3 value, QWidget *parent) :
    QWidget(parent),
    x_edit_(new DoubleEdit(value.x)),
    y_edit_(new DoubleEdit(value.y)),
    z_edit_(new DoubleEdit(value.z)),
    value_(value)
{
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    layout->addWidget(x_edit_);
    layout->addWidget(y_edit_);
    layout->addWidget(z_edit_);

    QSizePolicy policy;
    policy.setRetainSizeWhenHidden(true);
    x_edit_->setSizePolicy(policy);
    y_edit_->setSizePolicy(policy);
    z_edit_->setSizePolicy(policy);

    x_edit_->Changed.Connect(this, &Vec3Edit::OnXChanged);
    y_edit_->Changed.Connect(this, &Vec3Edit::OnYChanged);
    z_edit_->Changed.Connect(this, &Vec3Edit::OnZChanged);

    x_edit_->setToolTip("X");
    y_edit_->setToolTip("Y");
    z_edit_->setToolTip("Z");

    OnSetValue(value_);
}

void Vec3Edit::setDisabled(bool disabled) {
    x_edit_->setDisabled(disabled);
    y_edit_->setDisabled(disabled);
    z_edit_->setDisabled(disabled);
}

DoubleEdit &Vec3Edit::GetX() {
    return *x_edit_;
}

DoubleEdit &Vec3Edit::GetY() {
    return *y_edit_;
}

DoubleEdit &Vec3Edit::GetZ() {
    return *z_edit_;
}

void Vec3Edit::OnXChanged(double value) { value_.x = value; Changed.Emit(value_); }
void Vec3Edit::OnYChanged(double value) { value_.y = value; Changed.Emit(value_); }
void Vec3Edit::OnZChanged(double value) { value_.z = value; Changed.Emit(value_); }

// Update the UI
void Vec3Edit::OnSetValue(glm::vec3 value) {
    value_ = value;
    x_edit_->OnSetValue(value_.x);
    y_edit_->OnSetValue(value_.y);
    z_edit_->OnSetValue(value_.z);
    RedrawRequested.Emit();
}
