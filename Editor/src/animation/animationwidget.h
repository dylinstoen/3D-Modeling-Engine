/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ANIMATIONWIDGET_H
#define ANIMATIONWIDGET_H

#include <animator.h>
#include <QWidget>
#include <properties/property.h>

namespace Ui {
class AnimationWidget;
}

class MenuToolButton;
class QAnimatableWidgetItem;
class QSpinBox;
class Curve;
class CurveSamplerFactory;
class SceneObject;

class AnimationWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnimationWidget(QWidget *parent = 0);
    ~AnimationWidget();

    void ShowSceneObject(SceneObject* scene_object);
    bool ShowProperties(ObjectWithProperties* properties, QAnimatableWidgetItem* container);

    unsigned int GetAnimationLength() const { return animation_length_; }
    void SetAnimationLength(unsigned int t);
    unsigned int GetFPS() const { return fps_; }
    void SetFPS(unsigned int fps);

    void AnimationPlay() { PlayAnimation(); }
    void AnimationPause() { PauseAnimation(); }
    void AnimationSeek(float t) {
        frame_ = int(t * fps_);
        SeekAnimation(t);
    }

    CurveSamplerFactory& GetCurveFactory();

public slots:
    void Replot(Curve* curve=nullptr);

signals:
    void AnimationStart();
    void AnimationStop();
    void AnimationReset();
    void AnimationUpdate(float t);
    void FPSChanged(unsigned int fps);
    void AnilengthChanged(unsigned int anilength);
    void RealtimeChanged(bool set);

private:
    Ui::AnimationWidget* ui;
    QAnimatableWidgetItem* GetSelectedItem();
    std::set<Curve*> active_curves_;
    unsigned int frame_;

    // Widgets
    QSpinBox* anilength_spinbox;
    QSpinBox* fps_spinbox;

    // Actions
    MenuToolButton* curve_type_tool;
    QAction* linear_action;
    QAction* bezier_action;
    QAction* bspline_action;
    QAction* cmr_action;
    QAction* wrap_action;

    void ClearActiveCurves();
    void SetActiveProperty(Property* property);
    void SelectProperty(QAnimatableWidgetItem* prop_widget);
    void AddKeyframe(double t, QAnimatableWidgetItem* prop_widget);

    // Animation stuff
    bool looping_;
    bool realtime_;
    float tension_; // for catmull-rom
    unsigned int animation_length_; // in s
    unsigned int fps_;
    QTimer* animation_timer_;
    double current_time_ms_; // in ms
    std::chrono::high_resolution_clock::time_point previous_time_pt_;
    void PlayAnimation();
    void UpdateAnimation();
    void PauseAnimation();
    // Seek to time t in seconds
    void SeekAnimation(float t);

    // Slots
    void OnFrameChanged(float t, unsigned int frame);
    void OnRectangleSelect(const QRect&, QMouseEvent*);
    void OnItemSelectionChanged();
    void OnSetCmrTension();
    void OnSetAnimationLength();
    void OnSetFPS();
};

#endif // ANIMATIONWIDGET_H
