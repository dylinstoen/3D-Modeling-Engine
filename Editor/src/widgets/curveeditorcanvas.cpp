/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "curveeditorcanvas.h"
#include <QPen>
#include <QPainter>

CurveEditorCanvas::CurveEditorCanvas(QWidget *parent)
    : QWidget(parent),
      ctrl_points_(nullptr),
      draw_points_(nullptr),
      num_ctrl_points_(0),
      num_draw_points_(0),
      selected_index_(-1),
      extremes(5,5),
      dragging_(false)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void CurveEditorCanvas::paintEvent(QPaintEvent *event) {
    int point_size = 4;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2, height()/2);
    painter.setBrush(QBrush(Qt::green));
    double scale = ComputeScale();
    // Draw Axes
    painter.setPen(QPen(Qt::black));
    painter.drawLine(0, -5*scale, 0, 5*scale);
    painter.drawLine(-5*scale, 0, 5*scale, 0);
    // Draw Control Points
    for (size_t i = 0; i < num_ctrl_points_; i++) {
        int w = point_size;
        QColor color = Qt::black;
        if (i == selected_index_) {
            w *= 1.25;
            color = Qt::blue;
        } else if (i == 0) {
            w *= 1.25;
            color = Qt::red;
        }
        painter.fillRect(ctrl_points_[i].x()*scale - w,
                         ctrl_points_[i].y()*scale - w,
                         w*2, w*2, color);
    }
    // Draw Curve
    painter.setPen(QPen(Qt::red));
    painter.drawPolyline(draw_points_, num_draw_points_);
    // Draw reflected curve
    painter.scale(-1.f, 1.f);
    painter.setPen(QPen(Qt::green));
    painter.drawPolyline(draw_points_, num_draw_points_);
}
void CurveEditorCanvas::UpdatePoints(std::vector<glm::vec2> points) {
    num_draw_points_ = points.size();
    if (num_draw_points_ == 0) return;
    if (draw_points_) {
        delete [] draw_points_;
    }
    draw_points_ = new QPointF[num_draw_points_];

    dense_points_copy_.clear();
    dense_points_copy_.resize(num_draw_points_);
    if (!dragging_) { // don't explode in size when moving points
        extremes = glm::vec2(5,5);
        std::copy(points.begin(), points.end(), dense_points_copy_.begin());
        for (size_t i = 0; i < points.size(); i++) {
            extremes.x = std::max(std::abs(points[i].x), extremes.x);
            extremes.y = std::max(std::abs(points[i].y), extremes.y);
        }
    }
    double scale = ComputeScale();
    for (size_t i = 0; i < points.size(); i++) {
        draw_points_[i] = QPointF(points[i].x*scale, points[i].y*scale);
    }
    update();
}
void CurveEditorCanvas::UpdateControlPoints(std::vector<glm::vec2> points) {
    num_ctrl_points_ = points.size();
    if (num_ctrl_points_ == 0) return;
    if (ctrl_points_) {
        delete [] ctrl_points_;
    }
    ctrl_points_ = new QPointF[num_ctrl_points_];
    for (size_t i = 0; i < points.size(); i++) {
        ctrl_points_[i] = QPointF(points[i].x, points[i].y);
    }
    update();
}
void CurveEditorCanvas::SetSelectedIndex(size_t idx) {
    selected_index_ = idx;
    update();
}

void CurveEditorCanvas::mousePressEvent(QMouseEvent *event) {
    glm::vec2 p(event->x(), event->y());
    p.x -= width()/2;
    p.y -= height()/2;
    p /= ComputeScale();
    if (event->modifiers() & Qt::ControlModifier) {
        emit PointCreated(p.x, p.y, num_ctrl_points_);
        dragging_ = true;
    } else if (event->modifiers() & Qt::AltModifier) {
        int idx = findNearestDensePoint(p);
        if (idx >= 0) {
            emit PointCreated(p.x, p.y, idx);
            dragging_ = true;
        }
    } else if (event->modifiers() & Qt::ShiftModifier) {
        int idx = findNearestControlPoint(p);
        if (idx >= 0) emit PointDeleted(idx);
    } else {
        int idx = findNearestControlPoint(p);
        emit PointSelected(idx);
        if (idx >= 0) {
            dragging_ = true;
        }
    }
}

void CurveEditorCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (dragging_) {
        glm::vec2 p(event->x(), event->y());
        p.x -= width()/2;
        p.y -= height()/2;
        p /= ComputeScale();
        emit PointMoved(p.x, p.y, selected_index_);
    }
}

double pointToLineDistance(glm::vec2 start, glm::vec2 end, glm::vec2 p) {
    float d = glm::length(end - start);
    glm::vec2 v = (1.f/d)*(end - start);
    float t = glm::dot(p - start,v);
    if (t < 0) return glm::length(p - start);
    else if (t > d) return glm::length(p - end);
    else return glm::length(p - start - t*v);
}

int CurveEditorCanvas::findNearestDensePoint(glm::vec2 p, double maxdist) {
    if (num_ctrl_points_ < 2) return 0;
    double scale = ComputeScale();
    if (maxdist <= 0.f) {
        maxdist = devicePixelRatio()*5.f/scale;
    }
    double epsilon = 0.0001;
    double mindist = maxdist;
    int minidx = -1;
    size_t j = 1;
    glm::vec2 cp = glm::vec2(ctrl_points_[j].x(), ctrl_points_[j].y());
    for (size_t i = 0; i < dense_points_copy_.size() - 1; i++) {
        if (glm::distance(dense_points_copy_[i], cp) < epsilon) {
            j++;
            if (j < num_ctrl_points_) cp = glm::vec2(ctrl_points_[j].x(), ctrl_points_[j].y());
        }
        double d = pointToLineDistance(dense_points_copy_[i], dense_points_copy_[i+1], p);
        if (d < mindist) {
            mindist = d;
            minidx = j;
        }
    }
    return minidx;
}

int CurveEditorCanvas::findNearestControlPoint(glm::vec2 p, double maxdist) {
    double scale = ComputeScale();
    if (maxdist <= 0.f) {
        maxdist = devicePixelRatio()*10.f/scale;
    }
    double mindist = maxdist*maxdist;
    int minidx = -1;
    for (size_t i = 0; i < num_ctrl_points_; i++) {
        double dx = ctrl_points_[i].x() - p.x;
        double dy = ctrl_points_[i].y() - p.y;
        double d2 = dx*dx + dy*dy;
        if (d2 < mindist) {
            mindist = d2;
            minidx = i;
        }
    }
    return minidx;
}
