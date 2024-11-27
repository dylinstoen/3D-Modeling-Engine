/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <widgets/inspectablewidget.h>
#include <resource/asset.h>
#include <ColorPreview>
#include <ColorDialog>

using namespace color_widgets;

class ColorPicker : public QWidget, public InspectableWidget
{
    Q_OBJECT
public:
    Signal1<glm::vec4> Changed;

    ColorPicker(bool use_alpha = true, const glm::vec4& value = glm::vec4(1.0, 1.0, 1.0, 1.0), QWidget *parent = Q_NULLPTR);

public slots:
    void OnSetValue(glm::vec4);
protected:
    QHBoxLayout* layout_;
    ColorPreview* display_;
    ColorDialog* dialog_;
    QPushButton* dialog_btn_;
    AssetType type_;
    bool use_alpha_;
};

#endif // COLORPICKER_H
