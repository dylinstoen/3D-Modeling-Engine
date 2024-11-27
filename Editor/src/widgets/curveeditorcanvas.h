/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CURVEEDITORCANVAS_H
#define CURVEEDITORCANVAS_H

#include <QWidget>
#include <QPoint>
#include <vector>
#include <vectors.h>
#include <QMouseEvent>

class CurveEditorCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CurveEditorCanvas(QWidget *parent = nullptr);
    virtual void paintEvent(QPaintEvent *event);

    void UpdatePoints(std::vector<glm::vec2> points);
    void UpdateControlPoints(std::vector<glm::vec2> points);
    void SetSelectedIndex(size_t idx);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent*) { dragging_ = false; emit PointSelected(selected_index_); }
signals:
    void PointCreated(float x, float y, int idx);
    void PointDeleted(int idx);
    void PointSelected(int idx);
    void PointMoved(float x, float y, float idx);
public slots:

private:
    int findNearestDensePoint(glm::vec2 p, double maxdist=0.);
    int findNearestControlPoint(glm::vec2 p, double maxdist=0.);
    double ComputeScale() { return 0.8*std::min(width()/2/extremes.x, height()/2/extremes.y); }

    std::vector<glm::vec2> dense_points_copy_;
    QPointF* ctrl_points_;
    QPointF* draw_points_;
    size_t num_ctrl_points_;
    size_t num_draw_points_;
    size_t selected_index_;
    glm::vec2 extremes;

    bool dragging_;
};

#endif // CURVEEDITORCANVAS_H
