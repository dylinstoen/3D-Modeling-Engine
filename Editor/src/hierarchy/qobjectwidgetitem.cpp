/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "qobjectwidgetitem.h"
#include <resources.h>
#include <QGroupBox>
#include <QPushButton>
#include <inspector/inspector.h>

QObjectWidgetItem::QObjectWidgetItem(SceneObject& sceneObject) :
    QTreeWidgetItem(),
    InspectableItem(),
    props_dialog_(new PropertiesDialog()),
    scene_object_(&sceneObject),
    name_(sceneObject.GetName()),
    name_edit_(new LineEdit(sceneObject.GetName())),
    enabled_checkbox_(new CheckBox(sceneObject.IsEnabled()))
{
    // WidgetView
    QFrame* name_group = new QFrame();
    QHBoxLayout* name_layout = new QHBoxLayout();
    name_layout->setMargin(0);
    name_layout->addWidget(enabled_checkbox_);
    name_layout->addWidget(name_edit_);
    name_group->setLayout(name_layout);
    /*QPushButton* prop_edit_btn = new QPushButton(QObject::tr("Edit Properties"));
    QObject::connect(prop_edit_btn, &QPushButton::clicked, [this](){
        props_dialog_->exec(*properties_);
        PropertiesChanged.Emit();
    });*/
    layout_->addWidget(name_group);
    //layout_->addWidget(prop_edit_btn);

    QObject::connect(props_dialog_, &PropertiesDialog::PropertyChanged, [this](Property& prop, bool hidden){
        prop.SetHidden(hidden);
    });

    // Last thing in the layout and it expands to fill up the bottom
    QFrame* filler_group = new QFrame();
    QVBoxLayout* filler_layout = new QVBoxLayout();
    // QPushButton* add_component_btn = new QPushButton(QObject::tr("Add Component")); Leave this out for now
    // filler_layout->addWidget(add_component_btn);
    filler_layout->addStretch(); // Expands to push the button up to the top of the layout
    filler_group->setLayout(filler_layout);
    layout_->addWidget(filler_group);

    // Add each component in the scene object
    auto components = sceneObject.GetComponents();
    for (auto& component : components) {
        AddComponent(*component);
    }

    // Signals
    sceneObject.ComponentAdded.Connect(this, &QObjectWidgetItem::AddComponent);
    sceneObject.ComponentRemoved.Connect(this, &QObjectWidgetItem::RemoveComponent);
    sceneObject.ParentChanged.Connect(this, &QObjectWidgetItem::OnParentChanged);

    // Hook up enabled checkbox and renaming
    enabled_checkbox_->Changed.Connect(&sceneObject, &SceneObject::SetEnabled);
    sceneObject.EnabledChanged.Connect(enabled_checkbox_, &CheckBox::SetChecked);
    enabled_checkbox_->RedrawRequested.Connect(this, &InspectableItem::OnRedrawRequested);

    // Name editing
    setFlags(flags() | Qt::ItemIsEditable);
    name_edit_->Changed.Connect(&sceneObject, &SceneObject::SetName);
    sceneObject.NameChanged.Connect(this, &QObjectWidgetItem::OnNameChanged);

    // Initialize
    OnNameChanged(sceneObject.GetName());

    setHidden(sceneObject.IsHidden());
}

void QObjectWidgetItem::AddComponent(Component& component) {
    // TODO: Insert duplicate checks here
    int insertion_index = layout_->count() - 1; // Insert at 1 before the last widget (which is the filler)

    components_[component.GetTypeName()] = new QComponent(QString::fromStdString(component.GetTypeName()), *this);
    assert(components_[component.GetTypeName()] != nullptr);

    // Insert all the properties
    layout_->insertWidget(insertion_index, components_[component.GetTypeName()]);
    components_[component.GetTypeName()]->SetProperties(&component);
}

void QObjectWidgetItem::RemoveComponent(std::string classname) {
    components_[classname]->hide();
    layout_->removeWidget(components_[classname]);
    components_.erase(classname);
}

void QObjectWidgetItem::OnNameChanged(std::string name) {
    name_edit_->OnSetValue(name);
    name_ = name;
    setText(0, QString::fromStdString(name));
}

void QObjectWidgetItem::OnParentChanged(SceneObject& parent) {
    ParentChanged.Emit(*scene_object_, parent);
}

/*
void QObjectWidgetItem::OnHiddenChanged(bool hidden) {
    setHidden(hidden);
}
*/
