/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CURVEEDITORWINDOW_H
#define CURVEEDITORWINDOW_H

#include <QDialog>
#include <QCloseEvent>
#include <vector>
#include <vectors.h>

namespace Ui {
class CurveEditorWindow;
}

class CurveEditorWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CurveEditorWindow(QWidget *parent = 0);
    ~CurveEditorWindow();

    void Redraw();
    virtual void closeEvent(QCloseEvent* event);

    void SavePoints();

private:
    void ComputeDensePoints(std::vector<glm::vec2>& densepoints, int sample_rate);
    void CleanupDensePoints(std::vector<glm::vec2>& densepoints);
    void MakePointsRightHanded();

    Ui::CurveEditorWindow *ui;

    std::vector<glm::vec2> points_;
    int selected_index_;
    bool needs_save_;
    glm::vec2 save_scale_;
};

#endif // CURVEEDITORWINDOW_H
