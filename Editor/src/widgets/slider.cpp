/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "slider.h"
#include <QHBoxLayout>

Slider::Slider(double min, double max, double value, double step, QWidget *parent) :
    QWidget(parent),
    value_edit_(new DoubleEdit(value)),
    value_slide_(new QSlider(Qt::Horizontal))
{
    // Make sure that min < max
    if (max < min) max = min;
    // Make sure that the value lies between the min and max
    if (value < min) value = min;
    else if (value > max) value = max;

    // Setup the layout and add the widgets
    auto layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
    value_edit_->setMaximumWidth(50);
    value_slide_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    layout->addWidget(value_slide_);
    layout->addWidget(value_edit_);

    // Slider's hold integral values, so we need to perform a conversion.
    // Note that because it operates on integral values, we will display an approximation
    // of the decimal value, and store the correct value.
    double slider_total = max - min;
    int slider_ticks = std::ceil(slider_total / step);
    int slider_step = 1;

    range_min_ = min;
    slider_total_ = slider_total;
    slider_max_ = slider_ticks;

    value_slide_->setRange(0, slider_ticks);
    value_slide_->setSingleStep(slider_step);
    value_edit_->setSingleStep(step);

    connect(value_slide_, &QSlider::valueChanged, this, [this](int integral_value) {
        double aproximation = integral_value / slider_max_ * slider_total_ + range_min_;
        OnChanged(aproximation);
    });
    value_edit_->Changed.Connect(this, &Slider::OnChanged);

    OnSetValue(value);
}

void Slider::SliderSet(double value) {
    int integral_value = (value - range_min_) / slider_total_ * slider_max_;
    value_slide_->setValue(integral_value);
}

double Slider::SliderGet() {
    return value_;
}

void Slider::OnChanged(double value) {
    Changed.Emit(value);
}

// Update both the UI controls
void Slider::OnSetValue(double value) {
    // Suppress signals since we are just updating the UI
    const QSignalBlocker blocker1(value_slide_);
    const QSignalBlocker blocker2(value_edit_);
    SliderSet(value);
    value_edit_->OnSetValue(value);

    value_ = value;
    RedrawRequested.Emit();
}
