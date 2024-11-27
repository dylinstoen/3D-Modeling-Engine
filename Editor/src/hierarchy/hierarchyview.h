/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef HIERARCHYVIEW_H
#define HIERARCHYVIEW_H

#include <scene/scene.h>
#include <scene/renderer.h>
#include <scene/scenecamera.h>
#include <QObject>
#include <QMenu>
#include <QTreeWidget>
#include <hierarchy/qobjectwidgetitem.h>

// HierarchyView acts as a specialized QTreeWidget that uses QObjectWidgetItems instead.
// It acts as a view for the scene graph's SceneObjects and implemented SceneObserver to monitor changes.
class HierarchyView : public QTreeWidget {
    Q_OBJECT

public:
    HierarchyView();
    void SetScene(Scene& scene);
    SceneObject* GetSelectedSceneObject();
    void SelectObject(const SceneObject& scene_object);
public slots:
    void SelectObject(uint64_t object_id);
    void OnSceneNameChanged(const std::string& name);
    void OnObjectDuplicated(SceneObject& scene_object);
    void OnObjectDeleted(SceneObject& scene_object);
    void OnObjectCreated(SceneObject& scene_object);
    void OnObjectHideToggled(SceneObject& scene_object);
    void OnObjectParentChanged(SceneObject& scene_object, SceneObject& parent_object);
    void OnObjectPropertiesChanged();
    void OnChanged();
protected:
    // Maps from SceneObject uid to the QObjectWidgetItem
    QMenu* context_menu_;
    SceneObject* scene_root_;
    std::unordered_map<uint64_t, QObjectWidgetItem*> object_widgets_;
    QObjectWidgetItem& CreateWidgetItem(SceneObject& scene_object, QObjectWidgetItem *parent);
    QObjectWidgetItem* GetSelectedItem();
    void BuildSubtree(SceneObject& scene_node, QObjectWidgetItem *parent);
    virtual void dropEvent(QDropEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
signals:
    void InspectableSelected(InspectableItem* object_widget);
    void InspectableAdded(InspectableItem& object_widget);
    void InspectableRemoved(InspectableItem& object_widget);
    void ObjectParentChanged(uint64_t object_id, uint64_t parent_id);
    void ObjectSelected(uint64_t object_id);
    void ObjectPropertiesChanged(uint64_t object_id);
    void ObjectDuplicated(uint64_t object_id);
    void ObjectDeleted(uint64_t object_id);
    void ObjectCreated(uint64_t object_id);
    void ObjectFocused(uint64_t object_id);
    void HierarchyContextOpen(const QPoint& pos);
    void RedrawRequested();
    void AssetPickerRequested(AssetType, AssetPicker&);
};

#endif // HIERARCHYVIEW_H
