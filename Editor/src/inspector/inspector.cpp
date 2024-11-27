/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "inspector.h"
#include <widgets/inspectablewidget.h>

Inspector::Inspector() :
    blank_(new QWidget())
{
    addWidget(blank_);
}

void Inspector::OnAddInspectable(InspectableItem& item) {
    QWidget* item_widget = item.GetWidget();
    if (indexOf(item_widget) < 0) addWidget(item_widget);
}

void Inspector::OnRemoveInspectable(InspectableItem& item) {
    QWidget* item_widget = item.GetWidget();
    removeWidget(item_widget);
}

void Inspector::OnSelectInspectable(InspectableItem* item) {
    if (item != nullptr) {
        QWidget* item_widget = item->GetWidget();
        if (indexOf(item_widget) < 0) addWidget(item_widget);
        setCurrentWidget(item->GetWidget());
    } else {
        setCurrentWidget(blank_);
    }
}

void Inspector::AddProperty(QFormLayout& layout, std::string name, Property* property, InspectableItem &owner) {
    InspectableWidget* inspectable = nullptr;
    bool isHidden = property->IsHidden();
    QWidget* prop_widget = nullptr;
    QWidget* label = nullptr;
    layout.setVerticalSpacing(0);

    if (ResourcePropertyBase* prop = dynamic_cast<ResourcePropertyBase*>(property)) {
        auto resource = prop->GetAsset();
        std::string resource_name;
        if (resource != nullptr) resource_name = resource->GetName();
        AssetPicker* picker = new AssetPicker(resource_name, prop->GetAssetType());
        inspectable = picker;
        picker->AssetPickerRequested.Connect(&owner, &InspectableItem::OnAssetPickerRequested);
        layout.addRow(QString::fromStdString(name), picker);
        picker->Changed.Connect(prop, &ResourcePropertyBase::SetAsset);
        prop->ValueSet.Connect(picker, &AssetPicker::OnSetValue);
        prop_widget = picker;
        label = layout.labelForField(picker);
        if (resource != nullptr) resource->NameChanged.Connect(picker, &AssetPicker::OnAssetNameChanged);
    } else if (BooleanProperty* prop = dynamic_cast<BooleanProperty*>(property)) {
        CheckBox* checkbox = new CheckBox(prop->Get());
        inspectable = checkbox;
        layout.addRow(QString::fromStdString(name), checkbox);
        // When the Property changes, it should notify the scene.
        checkbox->Changed.Connect(prop, &BooleanProperty::Set);
        // Now the UI gets updates from the Scene, and then the UI updated (possibly the UI updates twice, can this create a cycle with some properties?).
        prop->ValueSet.Connect(checkbox, &CheckBox::SetChecked);
        prop_widget = checkbox;
        label = layout.labelForField(checkbox);
    } else if (IntProperty* prop = dynamic_cast<IntProperty*>(property)) {
        IntEdit* int_edit = new IntEdit(prop->Get());
        inspectable = int_edit;
        layout.addRow(QString::fromStdString(name), int_edit);
        int_edit->Changed.Connect(prop, &IntProperty::Set);
        prop->ValueChanged.Connect(int_edit, &IntEdit::OnSetValue);
        prop_widget = int_edit;
        label = layout.labelForField(int_edit);
    } else if (DoubleProperty* prop = dynamic_cast<DoubleProperty*>(property)) {
        if (prop->IsRange()) {
            Slider* slider = new Slider(prop->GetMin(), prop->GetMax(), prop->Get(), prop->GetStep());
            inspectable = slider;
            layout.addRow(QString::fromStdString(name), slider);
            slider->Changed.Connect(prop, &DoubleProperty::Set);
            prop->ValueChanged.Connect(slider, &Slider::OnSetValue);
            prop_widget = slider;
            label = layout.labelForField(slider);
        } else {
            DoubleEdit* double_edit = new DoubleEdit(prop->Get());
            inspectable = double_edit;
            layout.addRow(QString::fromStdString(name), double_edit);
            double_edit->Changed.Connect(prop, &DoubleProperty::Set);
            prop->ValueChanged.Connect(double_edit, &DoubleEdit::OnSetValue);
            prop_widget = double_edit;
            label = layout.labelForField(double_edit);
        }
    } else if (Vec3Property* prop = dynamic_cast<Vec3Property*>(property)) {
        Vec3Edit* vec3_edit = new Vec3Edit(prop->Get());
        inspectable = vec3_edit;
        layout.addRow(QString::fromStdString(name), vec3_edit);
        vec3_edit->Changed.Connect(prop, &Vec3Property::Set);
        prop->ValueChanged.Connect(vec3_edit, &Vec3Edit::OnSetValue);
        vec3_edit->setDisabled(prop->IsLocked());
        prop_widget = vec3_edit;
        label = layout.labelForField(vec3_edit);
        // Connect the Inner Double Edits
        prop->GetPropertyX().HiddenChanged.Connect(&vec3_edit->GetX(), &QWidget::setHidden);
        prop->GetPropertyY().HiddenChanged.Connect(&vec3_edit->GetY(), &QWidget::setHidden);
        prop->GetPropertyZ().HiddenChanged.Connect(&vec3_edit->GetZ(), &QWidget::setHidden);
        // Hide them if needed
        vec3_edit->GetX().setHidden(prop->GetPropertyX().IsHidden());
        vec3_edit->GetY().setHidden(prop->GetPropertyY().IsHidden());
        vec3_edit->GetZ().setHidden(prop->GetPropertyZ().IsHidden());
    } else if (ChoiceProperty* prop = dynamic_cast<ChoiceProperty*>(property)) {
        ComboBox* combobox = new ComboBox(prop->GetChoices(), prop->Get());
        inspectable = combobox;
        layout.addRow(QString::fromStdString(name), combobox);
        combobox->Changed.Connect(prop, &ChoiceProperty::Set);
        prop->ValueSet.Connect(combobox, &ComboBox::SetCurrentIndex);
        prop_widget = combobox;
        label = layout.labelForField(combobox);
    } else if (FileProperty* prop = dynamic_cast<FileProperty*>(property)) {
        FilePicker* picker = new FilePicker(prop->GetFileType(), prop->Get());
        inspectable = picker;
        layout.addRow(QString::fromStdString(name), picker);
        picker->Changed.Connect(prop, &FileProperty::Set);
        prop->ValueSet.Connect(picker, &FilePicker::OnSetValue);
        prop_widget = picker;
        label = layout.labelForField(picker);
    } else if (Mat4Property* prop = dynamic_cast<Mat4Property*>(property)) {
        // TODO: UI for matrices
    } else if (ColorProperty* prop = dynamic_cast<ColorProperty*>(property)) {
        ColorPicker* picker = new ColorPicker(prop->UsesAlpha(), prop->Get());
        inspectable = picker;
        layout.addRow(QString::fromStdString(name), picker);
        picker->Changed.Connect(prop, &ColorProperty::Set);
        prop->ValueSet.Connect(picker, &ColorPicker::OnSetValue);
        prop_widget = picker;
        label = layout.labelForField(picker);
    } else if (PropertyGroup* prop = dynamic_cast<PropertyGroup*>(property)) {
        layout.addRow("  ", (QWidget*)nullptr);
        QComponent* group = new QComponent(QString::fromStdString(name), owner);

        if (dynamic_cast<TextureProperty*>(prop)) {
            group->setFlat(true);
        }

        layout.addRow(group);
        prop->ValueSet.Connect(group, &QComponent::SetProperties);
        group->SetProperties(prop);
        if (isHidden) group->hide();
    } else {
        Debug::Log.WriteLine("UNKNOWN PROPERTY", Priority::Warning);
    }

    if (prop_widget != nullptr && label != nullptr) {
        if (isHidden) {
            prop_widget->hide();
            label->hide();
        }
        property->HiddenChanged.Connect(prop_widget, &QWidget::setHidden);
        property->HiddenChanged.Connect(label, &QWidget::setHidden);
    }

    if (inspectable != nullptr) {
        inspectable->RedrawRequested.Connect(&owner, &InspectableItem::OnRedrawRequested);
    }
}
