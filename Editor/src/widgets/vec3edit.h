/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QVECTOR3_H
#define QVECTOR3_H

#include <vectors.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <widgets/inspectablewidget.h>
#include <widgets/doubleedit.h>

class Vec3Edit : public QWidget, public InspectableWidget {
    Q_OBJECT
public:
    Vec3Edit(glm::vec3 value = glm::vec3(0.0, 0.0, 0.0), QWidget *parent = Q_NULLPTR);
    void setDisabled(bool disabled);

    DoubleEdit& GetX();
    DoubleEdit& GetY();
    DoubleEdit& GetZ();

    // Signals
    Signal1<glm::vec3> Changed;
public slots:
    void OnXChanged(double value);
    void OnYChanged(double value);
    void OnZChanged(double value);
    void OnSetValue(glm::vec3 value);
protected:
    DoubleEdit* x_edit_;
    DoubleEdit* y_edit_;
    DoubleEdit* z_edit_;
    glm::vec3 value_;
};

#endif // QVECTOR3_H
