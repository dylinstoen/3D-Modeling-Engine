/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QOBJECTWIDGETITEM_H
#define QOBJECTWIDGETITEM_H

#include <animator.h>
#include <QObject>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <widgets/qcomponent.h>
#include <inspector/inspectableitem.h>
#include <scene/sceneobject.h>
#include <widgets/checkbox.h>
#include <widgets/lineedit.h>
#include <widgets/propertiesdialog.h>

class QObjectWidgetItem : public QTreeWidgetItem, public InspectableItem {
public:
    QObjectWidgetItem(SceneObject& sceneObject);
    virtual std::string GetName() { return name_; }
    void AddComponent(Component& component);
    void RemoveComponent(std::string classname);
    SceneObject& GetSceneObject() { return *scene_object_; }

    Signal2<SceneObject&, SceneObject&> ParentChanged;
    Signal0<> PropertiesChanged;
protected:
    PropertiesDialog* props_dialog_;
    SceneObject* scene_object_; // Never null
    std::string name_;
    // Widget View
    LineEdit* name_edit_;
    CheckBox* enabled_checkbox_;
    // ?? whats this for exactly
    std::map<std::string, QComponent*> components_;
    void OnNameChanged(std::string name);
    void OnParentChanged(SceneObject& parent);
    //void OnHiddenChanged(bool hidden);
};

#endif // QOBJECTWIDGETITEM_H
