/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "animationwidget.h"
#include "ui_animationwidget.h"
#include <properties.h>
#include <scene/sceneobject.h>
#include <components.h>
#include <animation/qanimtablewidgetitem.h>
#include <widgets/menutoolbutton.h>
#include <animation/curve.h>

AnimationWidget::AnimationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnimationWidget),
    looping_(false),
    realtime_(false),
    tension_(0.5),
    animation_length_(1),
    fps_(30),
    animation_timer_(new QTimer(this))
{
    ui->setupUi(this);

    // Toolbar
    QMenu* curve_type_menu = new QMenu(tr("Curve Type"), this);
    linear_action = new QAction(tr("Linear"), this);
    bezier_action = new QAction(tr("Bezier Spline"), this);
    bspline_action = new QAction(tr("B-Spline"), this);
    cmr_action = new QAction(tr("Catmull-Rom Spline"), this);
    curve_type_menu->addAction(linear_action);
    curve_type_menu->addAction(bezier_action);
    curve_type_menu->addAction(bspline_action);
    curve_type_menu->addAction(cmr_action);

    wrap_action = new QAction(tr("Wrap Curve"), this);
    wrap_action->setCheckable(true);

    QAction* cmr_tension_action = new QAction(tr("Tension"), this);

    QAction* loop_action = new QAction(tr("Loop"), this);
    QAction* realtime_action = new QAction(tr("Play Realtime"), this);
    QAction* autokey_action = new QAction(tr("Auto-keyframe"), this);
    QAction* anilength_action = new QAction(tr("Set Length"), this);
    QAction* fps_action = new QAction(tr("Set FPS"), this);

    // Adds a keyframe for the currently selected properties
    QAction* keyframe_action = new QAction(tr("Keyframe"), this);
    keyframe_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    keyframe_action->setShortcut(Qt::Key_K);
    connect(keyframe_action, &QAction::triggered, this, [this]{
        QAnimatableWidgetItem* selected = GetSelectedItem();
        AddKeyframe(ui->curves_plot->GetCurrentTime(), selected);
    });

    cmr_tension_action->setVisible(false);

    // Looping
    loop_action->setCheckable(true);

    // Realtime play / Frame-by-Frame
    realtime_action->setCheckable(true);

    // Automatically keyframe on property change
    autokey_action->setCheckable(true);

    // Removes all currently selected keyframes
    QAction* remove_keyframe_action = new QAction(tr("Remove Keyframe"), this);
    remove_keyframe_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    QList<QKeySequence> del_shortcuts;
    del_shortcuts <<  Qt::Key_Backspace << QKeySequence::Delete;
    remove_keyframe_action->setShortcuts(del_shortcuts);
    connect(remove_keyframe_action, &QAction::triggered, ui->curves_plot, &CurvesPlot::DeleteControlPoints);

    // Rescale graph to fit all visible curves
    QAction* rescale_action = new QAction(tr("Rescale"), this);
    rescale_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    rescale_action->setShortcut(Qt::Key_F);
    connect(rescale_action, &QAction::triggered, ui->curves_plot, &CurvesPlot::Rescale);

    // Add actions to listen for shortcuts from this window
    addAction(keyframe_action);
    addAction(remove_keyframe_action);
    addAction(rescale_action);

    curve_type_tool = new MenuToolButton();
    curve_type_tool->setMenu(curve_type_menu);
    curve_type_tool->setDefaultAction(linear_action);

    QToolButton* wrap_tool = new QToolButton();
    wrap_tool->setDefaultAction(wrap_action);
    wrap_tool->setArrowType(Qt::NoArrow);

    QToolButton* cmr_tension_tool = new QToolButton();
    cmr_tension_tool->setDefaultAction(cmr_tension_action);

    QToolButton* loop_tool = new QToolButton();
    loop_tool->setDefaultAction(loop_action);

    QToolButton* realtime_tool = new QToolButton();
    realtime_tool->setDefaultAction(realtime_action);

    QToolButton* autokey_tool = new QToolButton();
    autokey_tool->setDefaultAction(autokey_action);

    QToolButton* keyframe_tool = new QToolButton();
    keyframe_tool->setDefaultAction(keyframe_action);

    QToolButton* anilength_tool = new QToolButton();
    anilength_tool->setDefaultAction(anilength_action);

    QToolButton* fps_tool = new QToolButton();
    fps_tool->setDefaultAction(fps_action);

    // Create and insert menubar into the layout
    QToolBar* menubar = new QToolBar(this);
    menubar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    menubar->setStyleSheet("QToolBar{spacing:5px;}");
    QVBoxLayout* vbox_layout = qobject_cast<QVBoxLayout*>(layout());
    assert(vbox_layout); // Must be vbox layout!
    vbox_layout->insertWidget(0, menubar);

    menubar->addWidget(curve_type_tool);
    menubar->addWidget(wrap_tool);
    menubar->addWidget(cmr_tension_tool);
    menubar->addWidget(loop_tool);
    menubar->addWidget(realtime_tool);
    menubar->addWidget(keyframe_tool);
    menubar->addWidget(autokey_tool);
    menubar->addWidget(anilength_tool);
    menubar->addWidget(fps_tool);

    connect(linear_action, &QAction::triggered, this, [this,cmr_tension_action](bool) {
        for (auto& curve : active_curves_) curve->SetCurveType(CurveType::Linear);
        cmr_tension_action->setVisible(false);
    });
    connect(bezier_action, &QAction::triggered, this, [this,cmr_tension_action](bool) {
        for (auto& curve : active_curves_) curve->SetCurveType(CurveType::Bezier);
        cmr_tension_action->setVisible(false);
    });
    connect(bspline_action, &QAction::triggered, this, [this,cmr_tension_action](bool) {
        for (auto& curve : active_curves_) curve->SetCurveType(CurveType::BSpline);
        cmr_tension_action->setVisible(false);
    });
    connect(cmr_action, &QAction::triggered, this, [this,cmr_tension_action](bool) {
        for (auto& curve : active_curves_) curve->SetCurveType(CurveType::CatmullRom);
        cmr_tension_action->setVisible(true);
    });
    connect(wrap_action, &QAction::triggered, this, [this](bool checked) {
        for (auto& curve : active_curves_) curve->SetWrapping(checked);
    });
    connect(cmr_tension_action, &QAction::triggered, this, &AnimationWidget::OnSetCmrTension);
    connect(loop_action, &QAction::triggered, this, [this](bool checked) {
        looping_ = checked;
    });
    connect(realtime_action, &QAction::triggered, this, [this](bool checked) {
        realtime_ = checked;
        frame_ = 0;
        emit RealtimeChanged(checked);
        SeekAnimation(0);
        emit AnimationReset();
    });
    connect(autokey_action, &QAction::triggered, this, [this](bool checked) {
        CurveSampler::AUTOKEY = checked;
    });

    connect(anilength_action, &QAction::triggered, this, &AnimationWidget::OnSetAnimationLength);
    connect(fps_action, &QAction::triggered, this, &AnimationWidget::OnSetFPS);
    connect(ui->curves_plot->selectionRect(), &QCPSelectionRect::accepted, this, &AnimationWidget::OnRectangleSelect);
    connect(ui->properties_view, &QTreeWidget::itemSelectionChanged, this, &AnimationWidget::OnItemSelectionChanged);

    // Animation
    // Precise timers try to keep millisecond accuracy
    animation_timer_->setTimerType(Qt::PreciseTimer);
    connect(animation_timer_, &QTimer::timeout, this, &AnimationWidget::UpdateAnimation);
    ui->play_button->setCheckable(true);
    connect(ui->play_button, &QPushButton::clicked, this, [this](bool playing) {
        if (playing) PlayAnimation();
        else PauseAnimation();
    });

    connect(ui->start_button, &QPushButton::clicked, this, [this]() {
       frame_ = 0;
       SeekAnimation(0);
       emit AnimationReset();
    });

    connect(ui->end_button, &QPushButton::clicked, this, [this]() {
       frame_ = animation_length_ * fps_;
       SeekAnimation(animation_length_);
    });

    // Hookup the slider
    connect(ui->seekbar, &QSlider::valueChanged, [this](int value){
        frame_ = value;
        SeekAnimation((float)value / fps_);
    });

    // Whenever the plot changes its point in time
    connect(ui->curves_plot, &CurvesPlot::FrameChanged, this, &AnimationWidget::OnFrameChanged);

    // Set Default FPS to 30 and animation length to 20 seconds
    SetFPS(30);
    SetAnimationLength(20);
    ui->curves_plot->replot();
}

AnimationWidget::~AnimationWidget() {
    delete ui;
}

void AnimationWidget::ShowSceneObject(SceneObject* scene_object) {
    ui->properties_view->clear();
    if (scene_object == nullptr) {
        ui->properties_view->setHeaderLabel("");
        ClearActiveCurves();
    } else {
        ui->properties_view->setHeaderLabel(QString::fromStdString(scene_object->GetName()));
        // Add all the animatable and non-hidden, non-locked properties to the list
        // TODO: Rather than checking for group properties, check for Vec3Property, and ColorProperty
        for (auto& component : scene_object->GetComponents()) {
            QAnimatableWidgetItem* container = new QAnimatableWidgetItem(component->GetTypeName(), nullptr);

            if (ShowProperties(component, container)) {
                ui->properties_view->addTopLevelItem(container);
                ClearActiveCurves();
            } else {
                delete container;
            }
        }
    }
}

bool AnimationWidget::ShowProperties(ObjectWithProperties* properties, QAnimatableWidgetItem* container) {
    bool has_props = false;
    for (auto& property_name : properties->GetProperties()) {
        auto property = properties->GetProperty(property_name);
        //if (property->IsHidden()) continue;
        DoubleProperty* animatable_prop = dynamic_cast<DoubleProperty*>(property);
        if (animatable_prop != nullptr) {
            has_props = true;
            QAnimatableWidgetItem* item = new QAnimatableWidgetItem(property_name, animatable_prop);
            item->PropertyUpdated.Connect(this, &AnimationWidget::Replot);
            container->addChild(item);
            SelectProperty(item);
        } else if (PropertyGroup* group_property = dynamic_cast<PropertyGroup*>(property)) {
            QAnimatableWidgetItem* group_container = new QAnimatableWidgetItem(property_name, nullptr);
            bool child_has_props = ShowProperties(group_property, group_container);
            if (child_has_props) {
                has_props = true;
                container->addChild(group_container);
            } else {
                delete group_container;
            }
        }
    }
    return has_props;
}

void AnimationWidget::Replot(Curve* curve) {
    ui->curves_plot->Replot(curve);
}

CurveSamplerFactory& AnimationWidget::GetCurveFactory() {
    return *ui->curves_plot;
}

void AnimationWidget::SetActiveProperty(Property *property) {
    // TODO: WIP
    if (property == nullptr) return;
}

QAnimatableWidgetItem *AnimationWidget::GetSelectedItem() {
    // TODO: Handle Multi-Selection
    // For now, just use the first thing in the selection
    QList<QTreeWidgetItem*> selected = ui->properties_view->selectedItems();
    if (selected.size() == 0) return nullptr;
    // Safe to dynamic cast to QPropertyWidgetItem since we only put those in
    return dynamic_cast<QAnimatableWidgetItem*>(selected[0]);
}

void AnimationWidget::ClearActiveCurves() {
    if (active_curves_.size() > 0) {
        for (auto& curve : active_curves_)
            curve->SetVisible(false);
        active_curves_.clear();
        Replot();
    }
}

void AnimationWidget::SelectProperty(QAnimatableWidgetItem *prop_widget) {
    if (prop_widget == nullptr) return;
    DoubleProperty* prop = prop_widget->GetProperty();
    if (prop == nullptr) {
        for(auto i = 0; i < prop_widget->childCount(); ++i) {
            QAnimatableWidgetItem* child = dynamic_cast<QAnimatableWidgetItem*>(prop_widget->child(i));
            assert(child != nullptr);
            SelectProperty(child);
        }
    } else {
        // Assume that if there's a curve sampler on the property, then it's a Curve type
        Curve* curve = dynamic_cast<Curve*>(prop->GetCurve());
        // If there's not already a curve on the property, create one
        if (curve == nullptr) {
            curve = &ui->curves_plot->AddCurve();
            curve->SetVisible(true);
            prop->SetCurve(curve);
        }
        // Set it as an active curve
        active_curves_.insert(curve);
        curve->SetVisible(true);
        // Update the curve toolbar (wrap and curve type)
        switch (curve->GetCurveType()) {
            case CurveType::Linear:
                curve_type_tool->setDefaultAction(linear_action);
                break;
            case CurveType::Bezier:
                curve_type_tool->setDefaultAction(bezier_action);
                break;
            case CurveType::BSpline:
                curve_type_tool->setDefaultAction(bspline_action);
                break;
            case CurveType::CatmullRom:
                curve_type_tool->setDefaultAction(cmr_action);
                break;
        }
        wrap_action->setChecked(curve->IsWrapping());
    }
}

void AnimationWidget::AddKeyframe(double t, QAnimatableWidgetItem *prop_widget) {
    if (prop_widget == nullptr) return;
    DoubleProperty* prop = prop_widget->GetProperty();
    if (prop == nullptr) {
        for(auto i = 0; i < prop_widget->childCount(); ++i) {
            QAnimatableWidgetItem* child = dynamic_cast<QAnimatableWidgetItem*>(prop_widget->child(i));
            assert(child != nullptr);
            AddKeyframe(t, child);
        }
    } else {
        // Assume that if there's a curve sampler on the property, then it's a Curve type
        Curve* curve = dynamic_cast<Curve*>(prop->GetCurve());
        // There should be no way this is nullptr because the property must be selected first
        assert(curve != nullptr);
        // Add keyframe to the curve
        curve->AddControlPoint(t, prop->Get());
    }
}

void AnimationWidget::SetFPS(unsigned int fps) {
    if (fps < 1) fps = 1;
    fps_ = fps;
    ui->curves_plot->SetFPS(fps);
    ui->seekbar->setRange(0, fps_ * animation_length_);
    emit FPSChanged(fps_);
}

void AnimationWidget::SetAnimationLength(unsigned int t) {
    if (t < 1) t = 1;
    animation_length_ = t;
    ui->curves_plot->SetAnimationLength(t);
    ui->seekbar->setRange(0, fps_ * animation_length_);
    emit AnilengthChanged(animation_length_);
}

void AnimationWidget::PlayAnimation() {
    previous_time_pt_ = std::chrono::high_resolution_clock::now();
    animation_timer_->start(17);
    emit AnimationStart();
}

void AnimationWidget::PauseAnimation() {
    animation_timer_->stop();
    emit AnimationStop();
    ui->play_button->setChecked(false);
    emit AnimationStop();
}

void AnimationWidget::UpdateAnimation() {
    // Calculate how much time has actually passed
    auto current_time_pt = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> delta_time_duration = current_time_pt - previous_time_pt_;
    double delta_time_ms = delta_time_duration.count();
    current_time_ms_ += delta_time_ms;
    double current_time_s_ = current_time_ms_ / 1000;
    previous_time_pt_ = current_time_pt;
    frame_++;

    if ((realtime_ && current_time_s_ >= animation_length_) ||
            (!realtime_ && frame_ > animation_length_ * fps_)) {
        frame_ = 0;
        // If we've reached the end, either stop the animation or loop
        if (looping_) {
            SeekAnimation(0);
            emit AnimationReset();
        }
        else PauseAnimation();
    } else {
        if (realtime_)
            SeekAnimation(current_time_s_);
        else
            SeekAnimation(frame_ / float(fps_));
    }
}

void AnimationWidget::SeekAnimation(float t) {
    // Ask the plot to change its point in time
    ui->curves_plot->MoveCursor(t);
}

void AnimationWidget::OnFrameChanged(float t, unsigned int frame) {
    const QSignalBlocker blocker(ui->seekbar);
    current_time_ms_ = t * 1000.0f;
    ui->seekbar->setValue(frame);
    ui->frame_display->setValue(frame);
    emit AnimationUpdate(frame / float(fps_));
}

void AnimationWidget::OnRectangleSelect(const QRect &rect, QMouseEvent *event) {
    Q_UNUSED(event)
    // Convert the rect from pixels to coordinates and normalize the rect of negative widths/heights
    float left = ui->curves_plot->xAxis->pixelToCoord(rect.left());
    float right = ui->curves_plot->xAxis->pixelToCoord(rect.left() + rect.width());
    if (right < left) std::swap(left, right);
    float width = right - left;
    float top = ui->curves_plot->yAxis->pixelToCoord(rect.top());
    float bottom = ui->curves_plot->yAxis->pixelToCoord(rect.top() + rect.height());
    if (top < bottom) std::swap(top, bottom);
    float height = top - bottom;

    QRectF coords(left, top, width, height);
    // Select the points that are contained inside the rect
    ui->curves_plot->deselectAll();
    for (auto& curve : active_curves_)
       for (auto& point : curve->GetControlPoints(coords))
           point->setSelected(true);
    ui->curves_plot->replot();
}

void AnimationWidget::OnItemSelectionChanged() {
    QAnimatableWidgetItem* selected = GetSelectedItem();
    if (selected) {
        ClearActiveCurves();
        SelectProperty(selected);
        ui->curves_plot->Rescale();
    }
}

void AnimationWidget::OnSetCmrTension() {
    bool okay;
    float tension = QInputDialog::getDouble(this, tr("Catmull-rom Scale Factor"), tr("Catmull-rom Tension"), tension_, 0.01, 3.0, 2, &okay, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (okay) {
        for (auto curve : active_curves_) {
            curve->SetTension(tension);
        }
        tension_ = tension;
    }
}

void AnimationWidget::OnSetAnimationLength() {
    bool okay;
    int ani_length = QInputDialog::getInt(this, tr("Animation Length"), tr("Length of Animation in Seconds"), animation_length_, 1, 60, 1, &okay, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (okay) SetAnimationLength(ani_length);
}

void AnimationWidget::OnSetFPS() {
    bool okay;
    int fps = QInputDialog::getInt(this, tr("FPS"), tr("Animation Frames Per Second"), fps_, 1, 60, 1, &okay, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (okay) SetFPS(fps);
}
