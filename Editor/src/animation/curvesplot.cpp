/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "curvesplot.h"

CurvesPlot::CurvesPlot(QWidget *parent) :
    QCustomPlot(parent),
    fixed_ticker_(new CustomTicker),
    frame_time_(1),
    cursor_coord_(0),
    moving_points_(false),
    hovered_point_(nullptr),
    frame_line_(new QCPItemStraightLine(this)),
    animation_length_(0),
    fps_(1)
{
    // Set the style of the frame line
    QPen frame_line_pen;
    frame_line_pen.setStyle(Qt::SolidLine);
    frame_line_pen.setWidth(1);
    frame_line_pen.setColor(Qt::red);
    frame_line_->setPen(frame_line_pen);
    frame_line_->setAntialiased(false);
    frame_line_->setClipToAxisRect(false);
    frame_line_->setSelectable(false);
    frame_line_->point1->setType(QCPItemPosition::PositionType::ptPlotCoords);
    frame_line_->point2->setType(QCPItemPosition::PositionType::ptPlotCoords);

    // Always start at time = 0
    MoveCursor(0);

    // Put it in the "overlay" layer
    QCPLayer* overlay_layer = layer("overlay");
    frame_line_->setLayer(overlay_layer);

    // Restrict the range of the xAxis
    connect(xAxis, static_cast<void (QCPAxis::*)(const QCPRange&)>(&QCPAxis::rangeChanged), this, [this](const QCPRange& range) {
        // Note: if we don't keep range.size, then x and y are scaled disproportionately
        double left_bound = 0;
        double right_bound = animation_length_;
        QCPRange boundedRange = range;
        if (boundedRange.lower < left_bound) {
            boundedRange.lower = left_bound;
            boundedRange.upper = std::min(range.size(), right_bound);
        }
        else if (boundedRange.upper > right_bound) {
            boundedRange.lower = std::max(right_bound - range.size(), left_bound);
            boundedRange.upper = right_bound;
        }
        xAxis->setRange(boundedRange);
    });

    // Margins
    axisRect()->setAutoMargins(QCP::msBottom);
    axisRect()->setMargins(QMargins(0,1,1,1));

    // Make left and bottom axes always transfer their ranges to right and top axes
//    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
//    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

//    QLinearGradient gradient(axisRect()->left(), 50, axisRect()->right(), 50);
    QLinearGradient gradient(0, 0, 0, 5);
    gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 1));
    gradient.setColorAt(0.25, QColor::fromRgbF(0, 0, 0, 0.5));
    gradient.setColorAt(0.5, QColor::fromRgbF(0, 0, 0, 0.25));
    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    QBrush brush(gradient);
    brush.setStyle(Qt::BrushStyle::LinearGradientPattern);
    QPen pen;  // creates a default pen
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(10);
    pen.setBrush(brush);
//    xAxis2->setBasePen(pen);
    // yAxis2->setBasePen(pen);

    xAxis2->setTicks(false);
    xAxis2->setSubTicks(false);
    xAxis2->setVisible(true);
    yAxis2->setTicks(false);
    yAxis2->setSubTicks(false);
    yAxis2->setVisible(true);

    // Axes
    fixed_ticker_->SetTickScaling(fps_);
    fixed_ticker_->setTickStep(1);
//    fixed_ticker_->setScaleStrategy(QCPAxisTickerFixed::ssMultiples);
    xAxis->setTicker(fixed_ticker_);
    xAxis->setRangeLower(0);
    yAxis->setRange(-10, 10);
    yAxis->setTickLabelSide(QCPAxis::lsInside);

    // User Interactions
    setMultiSelectModifier(Qt::ShiftModifier);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems | QCP::iMultiSelect);

    // Locale setting
    setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
}

void CurvesPlot::MoveCursor(float t, bool step) {
    // Restrict to valid time domain
    if (t < 0) t = 0;
    else if (t > animation_length_) t = animation_length_;

    // If step, round t to the nearest whole frame
    float frame_t = t;
    if (step) frame_t = RoundToStep(t);

    // Move the frame line
    frame_line_->point1->setCoords(frame_t, 0);
    frame_line_->point2->setCoords(frame_t, 1);
    cursor_coord_ = frame_t;

    replot();
    emit FrameChanged(t, std::round(frame_t * fps_));
}

void CurvesPlot::DeleteControlPoints() {
    std::set<Curve*> changed_curves;
    for (ControlPoint* ctrl_point : GetSelectedPoints()) {
        assert(ctrl_point != nullptr);
        Curve& curve = ctrl_point->GetParentCurve();
        curve.RemoveControlPoint(ctrl_point);
        changed_curves.insert(&curve);
    }
    for (auto& curve : changed_curves) curve->GenerateCurve();
    replot();
}

void CurvesPlot::mousePressEvent(QMouseEvent* event) {
    float bottom = axisRect()->bottom();
    float mouse_y = event->pos().y();
    if (mouse_y > bottom) {
        float mouse_x = event->pos().x();
        MoveCursor(xAxis->pixelToCoord(mouse_x));
    } else {
        setSelectionRectMode(QCP::SelectionRectMode::srmNone);
        axisRect()->setRangeDrag(0);

        // Begin moving points
        if (hovered_point_ != nullptr) {
            moving_points_ = true;
            float x_coord = RoundToStep(xAxis->pixelToCoord(event->localPos().x()));
            previous_mouse_coords_ = glm::vec2(x_coord, yAxis->pixelToCoord(event->localPos().y()));
            overlapped_points_.clear();
            // Force additive selection on if we're clicking on an already selected object,
            // in order to not deselect the other selected points.
            if (hovered_point_->selected()) event->setModifiers(event->modifiers() | Qt::ShiftModifier);
        } else {
            Qt::MouseButtons buttons = event->buttons();
            // LMB: select
            // LMB+Drag: rectangular select
            // LMB+Shift: multi-select
            // RMB+Drag: pan axes
            if (buttons & Qt::LeftButton) {
                setSelectionRectMode(QCP::SelectionRectMode::srmCustom);
            } else if (buttons & Qt::RightButton) {
                axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
            }
        }

        QCustomPlot::mousePressEvent(event);
    }
}

void CurvesPlot::mouseReleaseEvent(QMouseEvent* event) {
    // End moving points
    if (moving_points_) {
        moving_points_ = false;

        // Delete any overlapping points
        for (ControlPoint* ctrl_point : GetSelectedPoints()) {
            assert(ctrl_point != nullptr);
            Curve& current_curve = ctrl_point->GetParentCurve();
            ControlPoint* overlapped_point = current_curve.GetOverlappingPoint(ctrl_point);
            if (overlapped_point != nullptr) current_curve.RemoveControlPoint(overlapped_point);
        }
    }

    QCustomPlot::mouseReleaseEvent(event);
}

void CurvesPlot::mouseMoveEvent(QMouseEvent* event) {
    float bottom = axisRect()->bottom();
    float mouse_y = event->pos().y();
    if (mouse_y > bottom && event->buttons().testFlag(Qt::LeftButton)) {
        float mouse_x = event->pos().x();
        MoveCursor(xAxis->pixelToCoord(mouse_x));
    } else {
        if (moving_points_) {
            // Calculate how much to move each point in terms of coordinates
            float x_coord = RoundToStep(xAxis->pixelToCoord(event->localPos().x()));
            glm::vec2 current_mouse_coords = glm::vec2(x_coord, yAxis->pixelToCoord(event->localPos().y()));
            glm::vec2 delta_coords = current_mouse_coords - previous_mouse_coords_;
            previous_mouse_coords_ = current_mouse_coords;

            // Move all selected points
            auto selected_points = GetSelectedPoints();
            for (ControlPoint* ctrl_point : selected_points) {
                Curve& curve = ctrl_point->GetParentCurve();

                // Increment the control point, snap to nearest frame
                float x = RoundToStep(ctrl_point->Get().x + delta_coords.x);
                float y = ctrl_point->Get().y + delta_coords.y;
                // Restrict the domain
                if (x < 0) x = 0;
                else if (x > animation_length_) x = animation_length_;
                ctrl_point->Set(x, y);

                // Temporarily hide the overlapping control point unless we release
                ControlPoint* overlapped_point = curve.GetOverlappingPoint(ctrl_point);
                // If there was a different overlapped point before, show it now
                if (overlapped_points_.count(ctrl_point) > 0 && overlapped_point != overlapped_points_[ctrl_point]) {
                    curve.ShowControlPoint(overlapped_points_[ctrl_point]);
                    overlapped_points_.erase(overlapped_points_.find(ctrl_point));
                } else if (overlapped_point != nullptr && !overlapped_point->selected()) {
                    curve.HideControlPoint(overlapped_point);
                    overlapped_points_[ctrl_point] = overlapped_point;
                }

                // Redraw the curve
                curve.GenerateCurve();
            }

            // Fix for the case where user drags multiple points against the edge of the graph.
            // When they stack up, deleted the overlapping ones until there's one left (don't delete the hovered point!)
            std::set<ControlPoint*> pending_deletion;
            for (ControlPoint* ctrl_point : selected_points) {
                for (ControlPoint* other_point : selected_points) {
                    // Delete this point if it overlaps with any other point on the same curve and the other is not already being deleted
                    // This other point cannot be the same point, it cannot be on a different curve, and it also cannot be the point we're moving.
                    if (&ctrl_point->GetParentCurve() != &other_point->GetParentCurve() || ctrl_point == other_point || ctrl_point == hovered_point_) continue;
                    else if (pending_deletion.find(other_point) != pending_deletion.end()) continue;
                    if (std::abs(ctrl_point->Get().x - other_point->Get().x) < half_step_) {
                        pending_deletion.insert(ctrl_point);
                        break;
                    }
                }
            }
            for (ControlPoint* ctrl_point : pending_deletion) {
                ctrl_point->GetParentCurve().RemoveControlPoint(ctrl_point);
            }

            // Set the cursor to the point we clicked on to move
            // Don't step this value since the point might be on a half-frame
            MoveCursor(hovered_point_->Get().x, false);
        } else {
            ControlPoint* ctrl_point = qobject_cast<ControlPoint*>(itemAt(event->localPos(), true));
            if (ctrl_point != hovered_point_) hovered_point_ = ctrl_point;
        }

        QCustomPlot::mouseMoveEvent(event);
    }
}

void CurvesPlot::wheelEvent(QWheelEvent* event) {
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Scroll: Zoom Y-Axis
    // Scroll+Shift: Zoom
    // Scroll+Control: Zoom X-Axis
    if (modifiers & Qt::ShiftModifier)
        axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
    else if (modifiers & Qt::ControlModifier)
        axisRect()->setRangeZoom(xAxis->orientation());
    else
        axisRect()->setRangeZoom(yAxis->orientation());

    QCustomPlot::wheelEvent(event);
}

void CurvesPlot::Replot(Curve* curve) {
    if (curve) {
        curve->GenerateCurve();
    }
    this->replot();
}

void CurvesPlot::Rescale() {
    xAxis->rescale(true);
    yAxis->rescale(true);
    replot();
}

std::vector<ControlPoint*> CurvesPlot::GetSelectedPoints() {
    std::vector<ControlPoint*> points;
    for (auto& selected_item : selectedItems()) {
        if (ControlPoint* ctrl_pt = dynamic_cast<ControlPoint*>(selected_item)) {
            points.push_back(ctrl_pt);
        }
    }
    return points;
}
